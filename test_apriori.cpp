#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <mpi.h>

#include "dataset.h"
#include "compsup.h"
#include "apriori.h"


using namespace std;

#define _FILE_NAME "mnist_train_25.dat"
#define _TRANS_LEN 25
#define _MINSUP 30000
#define _NUM_THREADS 256



// Size of the MPI COMM
int g_mpiSize=1;

// Rank of the process in MPI
int g_mpiRank=0;



ostream& operator << (ostream &out, vector<size_t> items);


int main(int argc, char* argv[]){
    size_t trans_len=_TRANS_LEN;
    size_t minsup = _MINSUP;
    const char* file_name = _FILE_NAME;
    unsigned int num_threads = _NUM_THREADS;

    // initialize MPI
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &g_mpiSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &g_mpiRank); 

    cuda_init(g_mpiRank);

    ifstream file (file_name,ios::binary);

    if (!file.is_open()){
        cout<<"Failed to open file!!"<<endl;
        exit(-1);
    }

    istreambuf_iterator<etype> begin(file), end;
    dtype dat(begin,end);
    file.close();

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