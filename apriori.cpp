#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <mpi.h>

#include "dataset.h"
#include "compsup.h"
#include "apriori.h"
#include "getticks.h"

using namespace std;

#define _MPI_ELM_DTYPE MPI_CHAR
#define _MPI_SUP_DTYPE MPI_UNSIGNED 

#define _NUM_THREADS 256
#define _PFILE_NAME "patterns.dat"
#define _SFILE_NAME "supports.dat"

#define _DEL_EXISTING


int main(int argc, char* argv[]){
    size_t trans_len;
    size_t minsup;
    const char* file_name;
    const char* pfile_name=_PFILE_NAME;
    const char* sfile_name=_SFILE_NAME;
    unsigned int num_threads = _NUM_THREADS;
    
    if( argc < 4 || argc > 7 ){
        cout<<"apriori requires 3 arguments: file_name, transaction_length (in bytes) and minimum_support (unsigned integer) e.g. ./apriori mnist_25.dat 25 20000 \n";
        cout<<"Additionally you can provide 3 more arguments - num_threads, output_pattern_file, output_support_file e.g. ./apriori mnist_25.dat 25 20000 512 patterns.dat supports.dat \n";
        exit(-1);
    }

    
    file_name = argv[1];
    trans_len = (size_t)atoi(argv[2]);
    minsup = (size_t)atoi(argv[3]);
    

    // set num_threads if provided
    if( argc > 4 )
        num_threads = (unsigned int)atoi(argv[4]);
    
    // set output file names if provided
    if( argc > 5 )
        pfile_name = argv[5];

    if( argc > 6 )
        sfile_name = argv[6];
    

    MPI_File in_file;
    MPI_File pat_file;
    MPI_File sup_file;

    // Size of the MPI COMM
    int g_mpiSize=1;
    // Rank of the process in MPI
    int g_mpiRank=0;

    // initialize MPI
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &g_mpiSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &g_mpiRank); 


    if (MPI_File_open(MPI_COMM_WORLD, file_name, MPI_MODE_RDONLY, MPI_INFO_NULL, &in_file )) {
        cout<< "Unable to open input file!"<<endl;
        MPI_Finalize();
        exit(-1);
    }


    // Delete file if exists
    #ifdef _DEL_EXISTING
    MPI_File_open(MPI_COMM_WORLD, pfile_name,MPI_MODE_CREATE | MPI_MODE_WRONLY|MPI_MODE_DELETE_ON_CLOSE,MPI_INFO_NULL, &pat_file);
    MPI_File_open(MPI_COMM_WORLD, sfile_name,MPI_MODE_CREATE | MPI_MODE_WRONLY|MPI_MODE_DELETE_ON_CLOSE,MPI_INFO_NULL, &sup_file);
    MPI_File_close(&pat_file);
    MPI_File_close(&sup_file); 
    #endif


    if (MPI_File_open(MPI_COMM_WORLD, pfile_name,MPI_MODE_CREATE | MPI_MODE_WRONLY,
		                        MPI_INFO_NULL, &pat_file)){
        cout<< "Unable to open patterns file!"<<endl;
        MPI_Finalize();
        exit(-1);
    }

    if (MPI_File_open(MPI_COMM_WORLD, sfile_name,MPI_MODE_CREATE | MPI_MODE_WRONLY,
		                        MPI_INFO_NULL, &sup_file)){
        cout<< "Unable to open supports file!"<<endl;
        MPI_Finalize();
        exit(-1);
    }
    
    // Initialize CUDA and Compute
    cuda_init(g_mpiRank);
    Compute compute(trans_len, num_threads);


    // Read chunksize calculation
    MPI_Offset filesize;
    MPI_File_get_size(in_file, &filesize);

    size_t tot_trans = filesize/trans_len;
    size_t each_trans = (tot_trans+g_mpiSize-1)/g_mpiSize;

    size_t trans_start = each_trans*g_mpiRank;
    size_t trans_end = trans_start+each_trans;
    if (trans_end>tot_trans) trans_end=tot_trans;
    size_t trans_read = trans_end-trans_start;

    size_t file_start = trans_start*trans_len;
    size_t file_end = trans_end*trans_len;
    size_t file_read = file_end-file_start;


    // Read data
    compute.allocate_data(trans_read);

    MPI_File_read_at(in_file, (MPI_Offset)file_start, compute.get_data_addr(),
                             file_read, _MPI_ELM_DTYPE, MPI_STATUS_IGNORE);



    // Print details    
    if (g_mpiRank==0){   
        cout<<"=============================================="<<endl;
        cout<<"Input file: "<<file_name<<endl;
        cout<<"Output Files : Patterns -> "<<pfile_name<<"; Supports -> "<<sfile_name<<endl;
        cout<<"Transaction Length: "<<trans_len<<endl;
        cout<<"Total number of transactions: "<<tot_trans<<endl;
        cout<<"Minimum Support: "<<minsup<<endl;
        cout<<"Number of GPU Threads: "<<num_threads<<endl;
        cout<<"Number of MPI ranks: "<<g_mpiSize<<endl;
        cout<<"=============================================="<<endl;
    }



    //Initialize apriori
    Apriori apriori(trans_len, minsup);  
    size_t w_size,w_each,w_start,w_end,w_write, g_w_head=0;
    
    // time
    ticks t_write=0, t_compsup=0, t_mpi=0;
    ticks t_loop_start=getticks();
    do
    {   
        // Apriori step
        apriori.extend_tree();
        
        compute.set_patterns(apriori.patterns.get_data(), apriori.patterns.get_length());
        

        // Compute Support
        ticks t_compsup_start=getticks();
        compute.compute_support();
        ticks t_compsup_end=getticks();
        t_compsup+=t_compsup_end-t_compsup_start;

        compute.get_supports(apriori.supports.data());

        // MPI Allreduce
        ticks t_mpi_start=getticks();
        MPI_Allreduce(MPI_IN_PLACE,apriori.supports.data(), 
                        apriori.supports.size(), _MPI_SUP_DTYPE, 
                        MPI_SUM, MPI_COMM_WORLD);
        ticks t_mpi_end = getticks();
        t_mpi+=t_mpi_end-t_mpi_start;

        apriori.remove_infrequent();



        // Write patterns
        ticks t_write_start = getticks();

        w_size=apriori.supports.size();
        w_each=(w_size+g_mpiSize-1)/g_mpiSize;
        w_start=w_each*g_mpiRank;
        w_end=w_start+w_each;
        if (w_end>w_size) w_end=w_size;
        w_write=w_end-w_start;


        MPI_File_write_at(pat_file,(g_w_head+w_start)*trans_len, apriori.patterns.get_data()+w_start*trans_len,
                         w_write*trans_len, _MPI_ELM_DTYPE, MPI_STATUS_IGNORE);
        MPI_File_write_at(sup_file,(g_w_head+w_start)*sizeof(stype), apriori.supports.data()+w_start,
                            w_write*sizeof(stype), _MPI_ELM_DTYPE, MPI_STATUS_IGNORE);

        g_w_head+=w_size;
        
        ticks t_write_end = getticks();
        t_write += t_write_end-t_write_start;

    } while (apriori.patterns.get_length()>1);

    ticks t_loop_end=getticks();
    size_t t_loop=t_loop_end-t_loop_start;

    if (g_mpiRank==0) {
        cout<<"Number of frequent patterns = "<<g_w_head<<endl;
        cout<<"Time taken in main loop = "<<ticks_to_sec(t_loop)<<" sec"<<endl;
        cout<<"Time taken in compute_support = "<<ticks_to_sec(t_compsup)<<" sec"<<endl;
        cout<<"Time taken in Allreduce = "<<ticks_to_sec(t_mpi)<<" sec"<<endl;
        cout<<"Time taken in writing file = "<<ticks_to_sec(t_write)<<" sec"<<endl;
    }
    

    compute.free_all();
    
    MPI_File_close(&in_file);
    MPI_File_close(&pat_file);
    MPI_File_close(&sup_file);
    
    // Finalize MPI
    MPI_Finalize();
}
