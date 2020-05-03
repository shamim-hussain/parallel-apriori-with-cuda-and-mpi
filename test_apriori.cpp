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







ostream& operator << (ostream &out, vector<size_t> items);
std::ifstream::pos_type filesize(const char* filename);


int main(int argc, char* argv[]){
    size_t trans_len=_TRANS_LEN;
    size_t minsup = _MINSUP;
    const char* file_name = _FILE_NAME;
    unsigned int num_threads = _NUM_THREADS;
    
    MPI_File in_file;
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

    cuda_init(g_mpiRank);
    

    if (MPI_File_open(MPI_COMM_WORLD, file_name, MPI_MODE_RDONLY, MPI_INFO_NULL, &in_file )) {
        cout<< "Unable to open file!"<<endl;
        exit(-1);
    }

    
    MPI_File_get_size(in_file, &filesize);

    size_t tot_trans = filesize/trans_len;
    size_t each_trans = (tot_trans+g_mpiSize-1)/g_mpiSize;

    size_t trans_start = each_trans*g_mpiRank;
    size_t trans_end = trans_start+each_trans;
    if (trans_end>tot_trans) trans_end=tot_trans;
    size_t trans_read = trans_start-trans_end;

    size_t file_start = trans_start*trans_len;
    size_t file_end = trans_end*trans_len;
    size_t file_read = file_end-file_start;

    dtype dat(file_read);

    MPI_File_read_at(in_file, (MPI_Offset)file_start, dat.data(),
                             file_read, _MPI_ELM_DTYPE, &mpistat);

    MPI_File_close(&in_file);
    
    cout<<"File Size = "<<filesize<<endl;


    Dataset dataset(trans_len);
    dataset.swap_data(dat);



    Apriori apriori(trans_len, minsup);
    Compute compute(trans_len, num_threads);
    compute.set_data(dataset.get_data(),dataset.get_length());

    do
    {   
        cout<<"================================="<<endl;
        apriori.extend_tree();
        cout<<"Length of C"<<apriori.get_level()<<" = "<<apriori.patterns.get_length()<<endl;

        //out<<"Computing Support..."<<endl;
        compute.set_patterns(apriori.patterns.get_data(), apriori.patterns.get_length());
        compute.compute_support();
        compute.get_supports(apriori.supports.data());

        MPI_Allreduce(MPI_IN_PLACE,apriori.supports.data(), 
                        apriori.supports.size(), _MPI_SUP_DTYPE, 
                        MPI_SUM, MPI_COMM_WORLD);
                        
        apriori.remove_infrequent();
        cout<<"Length of F"<<apriori.get_level()<<" = "<<apriori.patterns.get_length()<<endl;

        cout<<"================================="<<endl;
        cout<<"Patterns and Supports:"<<endl;
        for(size_t i=0;i<apriori.patterns.get_length();i++){
            cout<<apriori.patterns[i].items()<<" - "<<apriori.supports[i]<<endl;
        }
        cout<<"================================="<<endl;
        cout<<endl;
    } while (apriori.patterns.get_length()>1);

    compute.free_all();
    
    // Finalize MPI
    MPI_Finalize();
}


ostream& operator << (ostream &out, vector<size_t> items){
    vector<size_t>::iterator i=items.begin();
    out<<*i;
    i++;
    for (;i!=items.end();i++){
        out<<" "<<*i;
    }
    return out;
}