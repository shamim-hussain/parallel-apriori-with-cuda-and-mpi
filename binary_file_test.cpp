#include <iostream>
#include <fstream>
#include "mining.h"

int main(int argc, char* argv[]){
    int trans_len=TRANS_LEN;
    char* buffer;

    ifstream file ("mnist_train_25.dat",ios::binary);

    if (!file.is_open()){
        cout<<"Failed to open file!!"<<endl;
        exit(-1);
    }

    istreambuf_iterator<etype> begin(file), end;
    dtype dat(begin,end);
    file.close();
    Dataset D(dat,trans_len);

    cout<<"The 25th byte is ";
    for (int i=7;i>=0;i--) cout<< ((D[0][24]>>i)&1);
    cout<<endl;


    cout<<"The 2nd transaction: {";
    auto items=D[1].items();
    for(auto i=items.begin();i<items.end();i++) {cout<<" ";cout<<*i;cout<<" ";}
    cout<<"}"<<endl;


    cout<<"Length of the dataset = "<<D.get_length()<<endl;
    return 0;
}

