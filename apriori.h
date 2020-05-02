#ifndef __APRIORI_H
#define __APRIORI_H

#include <vector>
#include "dataset.h"

typedef unsigned int stype;
typedef vector<stype> sstype;

class Apriori{
    unsigned level=0;
    size_t trans_len;
    

    void get_C1(){
        size_t pat_size=trans_len<<3;
        Dataset C1(trans_len,pat_size);
        for (unsigned i=0; i<pat_size;i++){
            Itemset I(trans_len);
            I.add_item(i);
            C1.push_back(I);
        }
        patterns.swap(C1);
        supports.resize(pat_size);
    }

    void get_C2(){
        size_t len=patterns.get_length();
        size_t pat_size=(len*(len-1))>>1;
        Dataset C2(trans_len,pat_size);
        for (unsigned i=0; i<len;i++){
            for (unsigned j=i+1;j<len;j++){
                C2.push_back(patterns[i]|patterns[j]);
            }
        }
        patterns.swap(C2);
        supports.resize(pat_size);
    }

    void get_CN(){
        
    }

    public:
    stype minsup;
    Dataset patterns;
    sstype supports;

    Apriori(size_t t_len, stype msup):trans_len(t_len),
                        minsup(msup),patterns(t_len){
    }

    inline void extend_tree(){
        if (!level){
            get_C1();
            level++;
        }
        else if (level==1){
            get_C2();
            level++;
        }
    }

    inline void remove_infrequent(){
        sstype newsup;
        Dataset newpat(trans_len);

        for (size_t i = 0; i<supports.size();i++){
            if (supports[i]>=minsup){
                newpat.push_back(patterns[i]);
                newsup.push_back(supports[i]);
            }
        }
        patterns.swap(newpat);
        supports.swap(newsup);
    }

    inline unsigned int get_level(){
        return level;
    }
};

#endif