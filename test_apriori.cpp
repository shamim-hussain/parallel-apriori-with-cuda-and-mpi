#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <mpi.h>

#include "dataset.h"
#include "compsup.h"
#include "apriori.h"


using namespace std;

#define _MPI_ELM_DTYPE MPI_CHAR
#define _MPI_SUP_DTYPE MPI_UNSIGNED 

#define _FILE_NAME "mnist_train_25.dat"
#define _TRANS_LEN 25
#define _MINSUP 30000
#define _NUM_THREADS 256




std::ifstream::pos_type filesize(const char* filename);


int main(int argc, char* argv[]){
    size_t trans_len=_TRANS_LEN;
    size_t minsup = _MINSUP;
    const char* file_name = _FILE_NAME;
    unsigned int num_threads = _NUM_THREADS;
    
    MPI_File in_file;
    MPI_File pat_file;
    MPI_File sup_file;

    MPI_Offset filesize;
    MPI_Status mpistat;

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
    if (MPI_File_open(MPI_COMM_WORLD, "patterns.dat",MPI_MODE_CREATE | MPI_MODE_WRONLY,
		                        MPI_INFO_NULL, &pat_file)){
        cout<< "Unable to open patterns file!"<<endl;
        MPI_Finalize();
        exit(-1);
    }

    if (MPI_File_open(MPI_COMM_WORLD, "supports.dat",MPI_MODE_CREATE | MPI_MODE_WRONLY,
		                        MPI_INFO_NULL, &sup_file)){
        cout<< "Unable to open supports file!"<<endl;
        MPI_Finalize();
        exit(-1);
    }
    cuda_init(g_mpiRank);

    Compute compute(trans_len, num_threads);
    

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

    compute.allocate_data(trans_read);

    MPI_File_read_at(in_file, (MPI_Offset)file_start, compute.get_data_addr(),
                             file_read, _MPI_ELM_DTYPE, &mpistat);



    Apriori apriori(trans_len, minsup);    

    do
    {   
        apriori.extend_tree();
        
        compute.set_patterns(apriori.patterns.get_data(), apriori.patterns.get_length());
        compute.compute_support();
        compute.get_supports(apriori.supports.data());

        MPI_Allreduce(MPI_IN_PLACE,apriori.supports.data(), 
                        apriori.supports.size(), _MPI_SUP_DTYPE, 
                        MPI_SUM, MPI_COMM_WORLD);
                        
        apriori.remove_infrequent();
    } while (apriori.patterns.get_length()>1);

    compute.free_all();
    
    MPI_File_close(&in_file);
    MPI_File_close(&pat_file);
    MPI_File_close(&sup_file);
    
    // Finalize MPI
    MPI_Finalize();
}
