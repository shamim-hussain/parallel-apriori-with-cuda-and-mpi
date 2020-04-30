#include <vector>
#include <stdexcept>
using namespace std;

#define TRANS_LEN 25


typedef char etype;
typedef vector<etype> dtype;
typedef dtype::iterator itype;


class Itemset{
    itype address;
    unsigned int length;
    dtype data;

    public:
    Itemset(itype t_address, unsigned int t_length): address(t_address), length(t_length) {}
    Itemset(itype begin, itype end):address(begin), length(end-begin) {}
    Itemset(unsigned int t_length):data(t_length), length(t_length){
        address=data.begin();
    }
    Itemset(dtype t_data):data(t_data){
        address=data.begin();
        length=data.size();
    }


    char operator [] (int i){
        if (i>=length){
            throw overflow_error("Itemset index out of bound!");
        }
        return address[i];
    }
    itype begin(){
        return address;
    }

    itype end(){
        return address+length;
    }

    unsigned int get_len(){
        return length;
    }

    vector<unsigned int> items(){
        vector<unsigned int> v;
        for (unsigned int i=0;i<length*8;i++){
            if ((address[i>>3]>>(i&7))&1) v.push_back(i);
        }
        return v;
    }
};

class Dataset{
    dtype data;
    unsigned int trans_len;

    public:
    Dataset(unsigned int transaction_length):trans_len(transaction_length){}

    Dataset(dtype data_buffer, unsigned int transaction_length):
                    data(data_buffer), trans_len(transaction_length) {}
    
    Dataset(itype begin,  itype end, unsigned int transaction_length):
                    data(begin, end), trans_len(transaction_length) {}

    Dataset(unsigned int transaction_length, unsigned int dataset_length):trans_len(transaction_length),data(trans_len*dataset_length){ }

    Itemset operator [] (int i){
        if (i*trans_len>=data.size()){
            throw overflow_error("Dataset index out of bound!");
        }
        return Itemset(data.begin()+(i*trans_len), trans_len);
    }

    void reserve(size_t items){
        data.reserve(items*trans_len);
    }


    unsigned int get_trans_len(){
        return trans_len;
    }

    unsigned int get_length(){
        return data.size()/trans_len;
    }

    etype* get_data(){
        return data.data();
    }
    size_t get_size(){
        return data.size();
    }
};