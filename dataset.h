#ifndef __DATASET_H
#define __DATASET_H

#include <vector>
#include <assert.h>
using namespace std;

#define TRANS_LEN 25


typedef char etype;
typedef vector<etype> dtype;
typedef dtype::iterator itype;


class Itemset{

    friend class Dataset;

    itype address;
    size_t length;
    dtype data;

    public:
    Itemset(itype t_address, size_t t_length): address(t_address), length(t_length) {}
    Itemset(itype begin, itype end):length(end-begin),data(begin,end){
        address=data.begin();
    }
    Itemset(size_t t_length):length(t_length), data(t_length, 0){
        address=data.begin();
    }
    Itemset(dtype t_data):data(t_data){
        address=data.begin();
        length=data.size();
    }


    inline char& operator [] (size_t i){
        assert (i<length);
        return address[i];
    }
    inline itype begin() const{
        return address;
    }

    inline itype end() const{
        return address+length;
    }

    Itemset operator &(Itemset I) const{
        Itemset ret(length);
        for (unsigned i=0;i<length;i++){
            ret[i]=address[i] & I.address[i];
        }
        return ret;
    }

    inline Itemset operator |(Itemset I) const{
        Itemset ret(length);
        for (unsigned i=0;i<length;i++){
            ret[i]=address[i] | I.address[i];
        }
        return ret;
    }

    inline Itemset operator ~() const{
        Itemset ret(length);
        for (itype a1=address,a2=ret.address;a1!=address+length;a1++,a2++){
            *a2=~*a1;
        }
        return ret;
    }

    inline bool operator >(Itemset I) const{
        for (unsigned i=length-1;i;i++){
            if(address[i] > I.address[i]) return true;
            if(address[i] < I.address[i]) return false;
        }
        return false;
    }

    inline bool operator <(Itemset I) const{
        for (unsigned i=length-1;i;i++){
            if(address[i] < I.address[i]) return true;
            if(address[i] > I.address[i]) return false;
        }
        return false;
    }

    inline bool operator == (Itemset I) const{
        for (unsigned i=0;i<length;i++){
            if(address[i] != I.address[i]) return false;
        }
        return true;
    }

    inline bool is_subset_of(Itemset I) const{
        int not_subset=0;
        for (itype a1=address,a2=I.address;a1!=address+length;a1++,a2++){
            not_subset = not_subset | (*a1 &(~*a2));
        }
        return !not_subset;
    }

    inline bool contains(size_t i) const{
        return (address[i>>3]>>(i&7))&1;
    }

    inline void add_item(size_t i){
        unsigned b= i>>3;
        address[b]=address[b]|(1<<(i&7));
    }

    inline size_t get_len(){
        return length;
    }

    inline size_t count_items(){
        size_t num_items=0;
        etype c;
        for (itype b=address;b!=address+length;b++){
            c=*b;
            num_items+=(c&1)+((c>>1)&1)+((c>>2)&1)+((c>>3)&1)
                            +((c>>4)&1)+((c>>5)&1)+((c>>6)&1)+((c>>7)&1);
        }
        return num_items;
    }

    vector<size_t> items() const{
        vector<size_t> v;
        for (size_t i=0;i<length*8;i++){
            if (this->contains(i)) v.push_back(i);
        }
        return v;
    }
};

class Dataset{
    dtype data;
    size_t trans_len;

    public:

    Dataset(size_t transaction_length):trans_len(transaction_length){}

    Dataset(dtype data_buffer, size_t transaction_length):
                    data(data_buffer), trans_len(transaction_length) {}
    
    Dataset(itype begin,  itype end, size_t transaction_length):
                    data(begin, end), trans_len(transaction_length) {}

    Dataset(size_t transaction_length, size_t dataset_length):
                                trans_len(transaction_length){
        data.reserve(trans_len*dataset_length);
    }

    inline Itemset operator [] (size_t i){
        assert (i*trans_len<data.size());
        return Itemset(data.begin()+(i*trans_len), trans_len);
    }

    inline void reserve(size_t items){
        data.reserve(items*trans_len);
    }

    inline size_t get_trans_len() const{
        return trans_len;
    }

    inline size_t get_length() const{
        return data.size()/trans_len;
    }

    inline etype* get_data(){
        return data.data();
    }
    inline size_t get_size() const{
        return data.size();
    }

    inline void push_back(const Itemset I){
        data.insert(data.end(), I.begin(),I.end());
    }

    inline Itemset pop_back(){
        Itemset back(data.end()-trans_len,data.end());
        data.resize(data.size()-trans_len);
        return back;
    }

    inline void swap(Dataset& D){
        data.swap(D.data);
    }
    inline void swap_data(dtype& d){
        data.swap(d);
    }
};

#endif