#include <iostream>
#include <fstream>
#include "dataset.h"
#include "compsup.h"
#include "apriori.h"

using namespace std;

#define _FILE_NAME "mnist_train_25.dat"
#define _TRANS_LEN 25
#define _MINSUP 20000
#define _NUM_THREADS 256


ostream& operator << (ostream &out, vector<size_t> items){
    auto i=items.begin();
    out<<*i;
    i++;
    for (;i!=items.end();i++){
        out<<" "<<*i;
    }
    return out;
}

int main(int argc, char* argv[]){
    size_t trans_len=_TRANS_LEN;
    size_t minsup = _MINSUP;
    const char* file_name = _FILE_NAME;
    unsigned int num_threads = _NUM_THREADS;

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
}