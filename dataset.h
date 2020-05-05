#ifndef __DATASET_H
#define __DATASET_H

#include <vector>
#include <assert.h>
using namespace std;

// elmentary byte datatype
typedef char etype;
// array of bytes datatype
typedef vector<etype> dtype;
// array iterator type
typedef dtype::iterator itype;

// #define count_bits(c) (c&1)+((c>>1)&1)+((c>>2)&1)+((c>>3)&1)+((c>>4)&1)+((c>>5)&1)+((c>>6)&1)+((c>>7)&1)
// Look up table to count bits in a byte in constant time
const size_t  count_bits[]={0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2,
 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3,
  4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2,
   3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5,
    5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3,
     4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 
     3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 2, 3, 3, 4, 3, 
     4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 3, 4,
      4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8};


// An Itemset contains multiple bytes which is a bitset presentation
class Itemset{

    friend class Dataset;

    // Starting address
    itype address;
    // Number of bytes
    size_t length;
    // Data - if allocated
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

    // indexing
    inline char& operator [] (size_t i){
        assert (i<length);
        return address[i];
    }

    // begin iterator
    inline itype begin() const{
        return address;
    }

    // end iterator
    inline itype end() const{
        return address+length;
    }

    // bitwise and -> set intersection
    Itemset operator &(Itemset I) const{
        Itemset ret(length);
        for (unsigned i=0;i<length;i++){
            ret[i]=address[i] & I.address[i];
        }
        return ret;
    }

    // bitwise or -> set union
    inline Itemset operator |(Itemset I) const{
        Itemset ret(length);
        for (unsigned i=0;i<length;i++){
            ret[i]=address[i] | I.address[i];
        }
        return ret;
    }

    // bitwise not
    inline Itemset operator ~() const{
        Itemset ret(length);
        for (itype a1=address,a2=ret.address;a1!=address+length;a1++,a2++){
            *a2=~*a1;
        }
        return ret;
    }

    // For binary search
    inline bool operator >(Itemset I) const{
        for (long i=length-1;i>=0;i++){
            if((unsigned char)address[i] > (unsigned char)I.address[i]) return true;
            if((unsigned char)address[i] < (unsigned char)I.address[i]) return false;
        }
        return false;
    }

    // for binary search
    inline bool operator <(Itemset I) const{
        for (long i=length-1;i>=0;i++){
            if((unsigned char)address[i] < (unsigned char)I.address[i]) return true;
            if((unsigned char)address[i] > (unsigned char)I.address[i]) return false;
        }
        return false;
    }

    // for binary search
    inline bool operator == (Itemset I) const{
        for (unsigned i=0;i<length;i++){
            if(address[i] != I.address[i]) return false;
        }
        return true;
    }


    // determine if subset -> set subset operation
    inline bool is_subset_of(Itemset I) const{
        int not_subset=0;
        for (itype a1=address,a2=I.address;a1!=address+length;a1++,a2++){
            not_subset = not_subset | (*a1 &(~*a2));
        }
        return !not_subset;
    }


    // Get matching number of bits in the beginning -> used for sibling determination
    inline size_t match_start(Itemset I){
        size_t bits=0;
        etype a1,a2,b1,b2; 
        for(unsigned i=0;i<length;i++){
            a1=address[i];
            a2=I.address[i];
            if (a1==a2){
                bits+=count_bits[(unsigned char)a1];
            } else {
                while (a1)
                {
                    b1=a1&1;
                    b2=a2&1;
                    if (b1!=b2) break;
                    bits+=b1;
                    a1>>=1;
                    a2>>=1;
                }
                break;
            }
        }
        return bits;
    }

    // Set contains operation if a bit is set
    inline bool contains(size_t i) const{
        return (address[i>>3]>>(i&7))&1;
    }

    // Add an item -> set a bit
    inline void add_item(size_t i){
        unsigned b= i>>3;
        address[b]=address[b]|(1<<(i&7));
    }

    // Get byte length
    inline size_t get_len(){
        return length;
    }

    // Count number of set bits
    inline size_t count_items(){
        size_t num_items=0;
        etype c;
        for (itype b=address;b!=address+length;b++){
            c=*b;
            num_items+=count_bits[(unsigned char)c];
        }
        return num_items;
    }

    // Get items as set of integers
    vector<size_t> items() const{
        vector<size_t> v;
        for (size_t i=0;i<length*8;i++){
            if (this->contains(i)) v.push_back(i);
        }
        return v;
    }
};


// Stores itemsets in a contiguos manner in the memory
class Dataset{
    // data -> vector of bytes
    dtype data;
    // byte length of each transaction
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

    // indexing
    inline Itemset operator [] (size_t i){
        assert (i*trans_len<data.size());
        return Itemset(data.begin()+(i*trans_len), trans_len);
    }

    // Reserve memory for later push_backs
    inline void reserve(size_t items){
        data.reserve(items*trans_len);
    }

    // Get transaction lengths in bytes
    inline size_t get_trans_len() const{
        return trans_len;
    }

    // get number of transactions
    inline size_t get_length() const{
        return data.size()/trans_len;
    }

    // get memory address of the data
    inline etype* get_data(){
        return data.data();
    }

    // get size of the data
    inline size_t get_size() const{
        return data.size();
    }


    // Push back an Itemset
    inline void push_back(const Itemset I){
        data.insert(data.end(), I.begin(),I.end());
    }

    // Pop back and Itemset
    inline Itemset pop_back(){
        Itemset back(data.end()-trans_len,data.end());
        data.resize(data.size()-trans_len);
        return back;
    }

    // Swap data
    inline void swap(Dataset& D){
        data.swap(D.data);
    }

    // Swap data
    inline void swap_data(dtype& d){
        data.swap(d);
    }
};

#endif