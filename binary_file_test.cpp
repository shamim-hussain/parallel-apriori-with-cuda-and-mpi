#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
using namespace std;

#define TRANS_LEN 25

class Transaction{
    char* address;
    unsigned int length;

    public:
    Transaction(char* t_address, unsigned int t_length): address(t_address), length(t_length) {}

    char operator [] (int i){
        if (i>=length){
            throw overflow_error("Transaction index out of bound!");
        }
        return *(address+i);
    }
    char* operator & (){
        return address;
    }
    unsigned int get_len(){
        return length;
    }
};

class Dataset{
    char* buffer;
    unsigned int trans_len;
    unsigned int data_len;

    public:
    Dataset(char* data_buffer, unsigned int dataset_length, unsigned int transaction_length):
                    buffer(data_buffer),data_len(dataset_length),trans_len(transaction_length) {}

    Dataset(unsigned int dataset_length, unsigned int transaction_length):data_len(dataset_length),trans_len(transaction_length){
        buffer=new char[data_len*trans_len];
    }

    Transaction operator [] (int i){
        if (i>=data_len){
            throw overflow_error("Dataset index out of bound!");
        }
        return Transaction(buffer+(i*trans_len), trans_len);
    }

    unsigned int get_trans_len(){
        return trans_len;
    }

    unsigned int get_data_len(){
        return data_len;
    }

    char* get_buffer_address(){
        return buffer;
    }

    ~Dataset(){
        delete buffer;
    }
};

int main(int argc, char* argv[]){
    ifstream file ("mnist_train_25.dat",ios::binary|ios::ate);
    int file_len;
    int trans_len=TRANS_LEN;
    char* buffer;

    if (file.is_open()){
        file_len = file.tellg();
        file.seekg(0,ios::beg);
        buffer = new char[file_len];
        file.read(buffer,file_len);
        file.close();
    } else {
        cout<<"Failed to open file!!"<<endl;
    }

    Dataset D(buffer,file_len/trans_len,trans_len);

    cout<<"The 25th byte is ";
    for (int i=7;i>=0;i--) cout<<((D[0][24]>>i)&1);
    cout<<endl;

    cout<<"Length of the file = "<<file_len<<endl;
    return 0;
}