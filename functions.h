#ifndef __FUNCTIONS_H
#define __FUNCTIONS_H

#include "dataset.h"
#include "apriori.h"

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

void compute_support(Dataset& P, Dataset& D, sstype &S){
    unsigned int sup;
    vector<unsigned int> supports(P.get_length());
    for (size_t j=0;j<P.get_length();j++){
        sup=0;
        for (size_t i=0;i<D.get_length();i++){
            if (P[j].is_subset_of(D[i])) sup++;
        }
        supports[j]=sup;
    }
    S.swap(supports);
}


// C version (for Neehal and Shoron)
void compute_support(char* patterns, size_t num_patterns, 
                        char*  dataset, size_t num_data,
                        size_t trans_len, unsigned int* supports)
{
    unsigned int sup_j, not_subset;
    char* pat_j;
    char* dat_i;

    size_t j,k;
    char* dataset_end=dataset+num_data*trans_len;
    // Outer loop - iterates over patterns
    for (j=0,pat_j=patterns; j<num_patterns; j++, pat_j+=trans_len){
        sup_j=0;

        // Inner loop - iterates over transactions
        for (dat_i=dataset; dat_i<dataset_end; dat_i+=trans_len){
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


// C version (for Neehal and Shoron)
// void compute_support(char* patterns, size_t num_patterns, 
//                         char*  dataset, size_t num_data,
//                         size_t trans_len, unsigned int* supports)
// {
//     int not_subset;
//     char* pat_j, *dat_i;
//     unsigned int* sup_j;
//     size_t k;

//     char* dataset_end=dataset+num_data*trans_len;
//     unsigned int* supports_end=supports+num_patterns;

//     // Inner loop - iterates over transactions
//     for (dat_i=dataset; dat_i<dataset_end; dat_i+=trans_len){
        
//         // Outer loop - iterates over patterns
//         for (sup_j=supports,pat_j=patterns; sup_j<supports_end; sup_j++, pat_j+=trans_len){
//             // Innermost loop - iterates over bytes
//             not_subset=0;
            
//             for (k=0; k<trans_len; k++){
//                 not_subset = not_subset | (pat_j[k]&(~dat_i[k]));
//             }

//             *sup_j=*sup_j+!not_subset;
//         }    
//     }
// }



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