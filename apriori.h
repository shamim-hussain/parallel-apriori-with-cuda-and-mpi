#ifndef __APRIORI_H
#define __APRIORI_H

#include <vector>
#include "dataset.h"

typedef unsigned int stype;
typedef vector<stype> sstype;

class Apriori{
    unsigned level;
    size_t trans_len;
    Dataset* patterns;
    sstype* supports;
    stype minsup;

    void get_C1(){
        Dataset C1(trans_len,trans_len<<3);
        for (unsigned i=0; i<(trans_len<<3);i++){
            Itemset I(trans_len);
            I.add_item(i);
            C1.push_back(I);
        }
        patterns->swap(C1);
    }

    public:
    Apriori(Dataset* pat, sstype* sups, stype msup, unsigned lvl=0):
                level(lvl), trans_len(pat->get_trans_len()), patterns(pat),
                supports(sups), minsup(msup){
    }

    inline void extend_tree(){
        if (!level){
            get_C1();
            level++;
        }
    }

    inline void remove_infrequent(){
        
    }

    inline unsigned int get_level(){
        return level;
    }
};

#endif