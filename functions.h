#ifndef __FUNCTIONS_H
#define __FUNCTIONS_H

#include "dataset.h"

unsigned int compute_support(Itemset& x, Dataset& D){
    unsigned int sup=0;
    for (size_t i=0;i<D.get_length();i++){
        if (x.is_subset_of(D[i])) sup++;
    }
    return sup;
}

vector<unsigned int> compute_support(Dataset& P, Dataset& D){
    unsigned int sup;
    vector<unsigned int> supports(P.get_length());
    for (size_t j=0;j<P.get_length();j++){
        sup=0;
        for (size_t i=0;i<D.get_length();i++){
            if (P[j].is_subset_of(D[i])) sup++;
        }
        supports[j]=sup;
    }
    return supports;
}

// C version (for Neehal and Shoron)
void compute_support(char* patterns, size_t num_patterns, 
                        char*  dataset, size_t num_data,
                        size_t trans_len, unsigned int* supports)
{
    unsigned int sup_j, not_subset;
    char* pat_j;
    char* dat_i;

    size_t i,j,k;

    // Outer loop - iterates over patterns
    for (j=0,pat_j=patterns; j<num_patterns; j++, pat_j+=trans_len){
        sup_j=0;

        // Inner loop - iterates over transactions
        for (i=0,dat_i=dataset; i<num_data; i++, dat_i+=trans_len){
            not_subset=0;

            // Innermost loop - iterates over bytes
            for (k=0; k<trans_len; k++){
                not_subset = not_subset | (pat_j[k]&(~dat_i[k]));
            }

            sup_j = sup_j + !not_subset;
        }

        supports[j]=sup_j;
    }
}


Dataset get_C1(size_t trans_len){
    Dataset C1(trans_len);
    for (unsigned i=0; i<(trans_len<<3);i++){
        Itemset I(trans_len);
        I.add_item(i);
        C1.push_back(I);
    }
    return C1;
}

#endif