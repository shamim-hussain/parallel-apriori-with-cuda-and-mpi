/*
Main entry point file
*/

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <mpi.h>

#include "dataset.h"
#include "compsup.h"
#include "apriori.h"
#include "getticks.h"

using namespace std;

// MPI datatypes used
#define _MPI_ELM_DTYPE MPI_CHAR
#define _MPI_SUP_DTYPE MPI_UNSIGNED 

// Default number or threads
#define _NUM_THREADS 256
// Default patterns and supports output file names
#define _PFILE_NAME "patterns.dat"
#define _SFILE_NAME "supports.dat"

// Delete file if exists?
#define _DEL_EXISTING

// Default read and write block sizes
#define _RD_BLK_SIZE (1024<<6) //64KB
#define _WR_BLK_SIZE (1024<<6)



int main(int argc, char* argv[]){
    // Transaction length in byts
    size_t trans_len;
    // Minimum support
    size_t minsup;
    // Input file name
    const char* file_name;
    // Output patterns and supports files
    const char* pfile_name=_PFILE_NAME;
    const char* sfile_name=_SFILE_NAME;
    // Number of cuda threads
    unsigned int num_threads = _NUM_THREADS;

    // Read and write block sizes
    size_t rd_blk_size = _RD_BLK_SIZE;
    size_t wr_blk_size = _WR_BLK_SIZE;
    
    if( argc < 4 || argc > 7 ){
        cout<<"apriori requires 3 arguments: file_name, transaction_length (in bytes) and minimum_support (unsigned integer) e.g. ./apriori mnist_25.dat 25 20000 \n";
        cout<<"Additionally you can provide 3 more arguments - num_threads, read block size, write block size e.g. ./apriori mnist_25.dat 25 20000 512 patterns.dat supports.dat \n";
        exit(-1);
    }

    
    file_name = argv[1];
    trans_len = (size_t)atoi(argv[2]);
    minsup = (size_t)atoi(argv[3]);
    

    // set num_threads if provided
    if( argc > 4 )
        num_threads = (unsigned int)atoi(argv[4]);
    
    if( argc > 5 )
        rd_blk_size = (size_t)atoi(argv[5]);

    if( argc > 6 )
        wr_blk_size = (size_t)atoi(argv[6]);

    // set output file names if provided
    // if( argc > 7 )
    //     pfile_name = argv[7];

    // if( argc > 8 )
    //     sfile_name = argv[8];
    

    //Mpi file handles
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

    // Open input file
    if (MPI_File_open(MPI_COMM_WORLD, file_name, MPI_MODE_RDONLY, MPI_INFO_NULL, &in_file )) {
        cout<< "Unable to open input file!"<<endl;
        MPI_Finalize();
        exit(-1);
    }


    // Delete output files if exists
    #ifdef _DEL_EXISTING
    MPI_File_open(MPI_COMM_WORLD, pfile_name,MPI_MODE_CREATE | MPI_MODE_WRONLY|MPI_MODE_DELETE_ON_CLOSE,MPI_INFO_NULL, &pat_file);
    MPI_File_open(MPI_COMM_WORLD, sfile_name,MPI_MODE_CREATE | MPI_MODE_WRONLY|MPI_MODE_DELETE_ON_CLOSE,MPI_INFO_NULL, &sup_file);
    MPI_File_close(&pat_file);
    MPI_File_close(&sup_file); 
    #endif

    // Open output files
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
    
    // Initialize CUDA and a Compute class object
    cuda_init(g_mpiRank);
    Compute compute(trans_len, num_threads);


    // Get file size and total number of transactions
    MPI_Offset filesize;
    MPI_File_get_size(in_file, &filesize);
    size_t tot_trans = filesize/trans_len;
    // Determine read and write transactions block size
    size_t read_per_iter=(rd_blk_size+trans_len-1)/trans_len;
    size_t write_per_iter=(wr_blk_size+trans_len-1)/trans_len;


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
        cout<<"Read block size per rank: "<<rd_blk_size<<" Bytes"<<endl;
        cout<<"Read block size per rank: "<<read_per_iter<<" transactions"<<endl;
        cout<<"Write block size per rank: "<<wr_blk_size<<" Bytes"<<endl;
        cout<<"Write block size per rank: "<<write_per_iter<<" transactions"<<endl;
        cout<<"=============================================="<<endl;
    }



    // Initialize apriori
    Apriori apriori(trans_len, minsup);  

    // Allocate read memory
    compute.allocate_data(read_per_iter);

    // Read and write heads
    size_t w_size,w_each,w_start,w_end,w_write,g_w_end;
    size_t g_w_head=0;
    size_t r_start,r_end,r_read, g_r_head=0;

    // time
    ticks t_write=0, t_compsup=0, t_read=0; t_write=0;
    size_t i_ter=0;
    ticks t_loop_start=getticks();
    do
    {   
        
        // Apriori candidate generation step
        apriori.extend_tree();

        // No patterns generated end the algorithm
        if(apriori.patterns.get_length()==0) break;

        // Increase number of iterations
        i_ter++;

        // Transfer candidate patterns to compute (from apriori)
        compute.set_patterns(apriori.patterns.get_data(), apriori.patterns.get_length());

        // Read chunks of input data from file and compute support
        for(g_r_head=0;g_r_head<tot_trans;g_r_head+=read_per_iter*g_mpiSize){
            r_start=g_r_head+read_per_iter*g_mpiRank;
            r_end=r_start+read_per_iter;
            if (r_end>tot_trans) r_end=tot_trans;
            r_read=r_end-r_start;

            //Read from file
            ticks t_read_start=getticks();
            MPI_File_read_at(in_file, r_start*trans_len, compute.get_data_addr(),
                                r_read*trans_len, _MPI_ELM_DTYPE, MPI_STATUS_IGNORE);
            ticks t_read_end=getticks();
            t_read+=t_read_end-t_read_start;

            // Set size of read data
            compute.set_num_data(r_read);
            
            // Compute Support
            ticks t_compsup_start=getticks();
            compute.compute_support();
            ticks t_compsup_end=getticks();
            t_compsup+=t_compsup_end-t_compsup_start;            
        }

        // Transfer computed support values back to apriori (from compute)
        compute.get_supports(apriori.supports.data());

        // MPI Allreduce
        ticks t_mpi_start=getticks();
        MPI_Allreduce(MPI_IN_PLACE,apriori.supports.data(), 
                        apriori.supports.size(), _MPI_SUP_DTYPE, 
                        MPI_SUM, MPI_COMM_WORLD);
        ticks t_mpi_end = getticks();
        t_write+=t_mpi_end-t_mpi_start;
        
        //Remove infrequent patterns
        apriori.remove_infrequent();



        // Write patterns
        w_size=apriori.supports.size();

        g_w_end=g_w_head+w_size;
        for(size_t w_head=g_w_head;w_head<g_w_end;w_head+=write_per_iter*g_mpiSize){
            w_start=w_head+write_per_iter*g_mpiRank;
            w_end=w_start+write_per_iter;
            if (w_end>g_w_end) w_end=g_w_end;
            w_write=w_end-w_start;
            

            ticks t_write_start = getticks();
            MPI_File_write_at(pat_file,w_start*trans_len, apriori.patterns.get_data()+(w_start-g_w_head)*trans_len,
                         w_write*trans_len, _MPI_ELM_DTYPE, MPI_STATUS_IGNORE);
            MPI_File_write_at(sup_file,w_start*sizeof(stype), apriori.supports.data()+(w_start-g_w_head),
                            w_write*sizeof(stype), _MPI_ELM_DTYPE, MPI_STATUS_IGNORE);
            ticks t_write_end = getticks();
            t_write += t_write_end-t_write_start;
        }
        g_w_head=g_w_end;
        
    } while (apriori.patterns.get_length()>1);

    ticks t_loop_end=getticks();
    size_t t_loop=t_loop_end-t_loop_start;
    
    // Print out timings
    if (g_mpiRank==0) {
        cout<<"Number of frequent patterns = "<<g_w_head<<endl;
        cout<<"Time taken in main loop = "<<ticks_to_sec(t_loop)<<" sec"<<endl;
        cout<<"Time taken in compute_support = "<<ticks_to_sec(t_compsup)<<" sec"<<endl;
        cout<<"Time taken in Allreduce = "<<ticks_to_sec(t_write)<<" sec"<<endl;
        cout<<"Apriori iterations = "<<i_ter<<endl;
        cout<<"Time taken in reading file = "<<ticks_to_sec(t_read)<<" sec"<<endl;
        cout<<"Time taken in writing file = "<<ticks_to_sec(t_write)<<" sec"<<endl;
    }
    
    // Clean up and finalize
    compute.free_all();
    
    MPI_File_close(&in_file);
    MPI_File_close(&pat_file);
    MPI_File_close(&sup_file);
    
    // Finalize MPI
    MPI_Finalize();
}
