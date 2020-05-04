#include <iostream>
#include <fstream>
#include <vector>
#include "../dataset.h"

using namespace std;

#define _SUP_FILE_NAME "supports.dat"
#define _PAT_FILE_NAME "patterns.dat"
#define _TRANS_LEN 25



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
    const char* pat_file_name = _PAT_FILE_NAME;
    const char* sup_file_name = _SUP_FILE_NAME;

    if (argc==3){
        pat_file_name=argv[1];
        sup_file_name=argv[2];
    }

    ifstream pfile (pat_file_name,ios::binary);

    if (!pfile.is_open()){
        cout<<"Failed to open pattern file!!"<<endl;
        exit(-1);
    }

    istreambuf_iterator<etype> pbegin(pfile), pend;
    dtype dat(pbegin,pend);
    pfile.close();

    Dataset patterns(trans_len);
    patterns.swap_data(dat);



    ifstream sfile (sup_file_name,ios::binary);

    if (!sfile.is_open()){
        cout<<"Failed to open support file!!"<<endl;
        exit(-1);
    }

    istreambuf_iterator<char> ubegin(sfile), uend;
    vector<char> sup(ubegin,uend);
    sfile.close();

    // vector<unsigned int> supports;
    // for (size_t i=0;i<patterns.get_length;i++)
    unsigned int* supports=(unsigned int*)(sup.data());


    vector <unsigned int> FN;

    cout<<"===================================================="<<endl;
    cout<<"Frequent Patterns and Supports:"<<endl;

    for(size_t i=0;i<patterns.get_length();i++){
        vector<size_t> items=patterns[i].items();

        if (items.size()>FN.size())
            FN.push_back(1);
        else
            FN.back()++;

        cout<<items<<" - "<<supports[i]<<endl;
    }

    cout<<"===================================================="<<endl;
    cout<<"Number of Patterns of Different Lengths:"<<endl;
    for (size_t i=0; i<FN.size(); i++){
        cout<<"F"<<(i+1)<<" : "<<FN[i]<<endl;
    }
    cout<<"===================================================="<<endl;
}