#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "compsup.h"

// w/o-CUDA version
void compute_support_proc(char* patterns, size_t num_patterns, 
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

        supports[j]+=sup_j;
    }
}



void Compute::set_data(char*  dataset, size_t num_data){
	g_num_data=num_data;
	if (g_dataset!=NULL) free(g_dataset);
	g_dataset=(char*)malloc(g_num_data * g_trans_len * sizeof(char));
	memcpy(g_dataset, dataset, g_num_data * g_trans_len * sizeof(char));
}

void Compute::allocate_data(size_t num_data){
	g_num_data=num_data;
	if (g_dataset!=NULL) free(g_dataset);
	g_dataset=(char*)malloc(g_num_data * g_trans_len * sizeof(char));
}

char* Compute::get_data_addr(){
	return g_dataset;
}

void Compute::set_patterns(char* patterns, size_t num_patterns){
	g_num_patterns=num_patterns;
	if (g_patterns!=NULL) free(g_patterns);				
	g_patterns=(char*)malloc(g_num_patterns * g_trans_len * sizeof(char)); 
	memcpy(g_patterns, patterns, g_num_patterns * g_trans_len * sizeof(char));

	if (g_supports!=NULL) free(g_supports);
	g_supports=(unsigned int*)malloc(g_num_patterns * sizeof(unsigned int));
	memset(g_supports,0,g_num_patterns * sizeof(unsigned int));
}

void Compute::compute_support(){         
	compute_support_proc(g_patterns, g_num_patterns,g_dataset, g_num_data, 
						g_trans_len, g_supports);
}


void Compute::get_supports(unsigned int* supports){
	memcpy(supports, g_supports, g_num_patterns * sizeof(unsigned int));
}

void Compute::free_all(){
	if (g_dataset!=NULL) free(g_dataset);
	if (g_patterns!=NULL) free(g_patterns);
	if (g_supports!=NULL) free(g_supports);
	g_dataset=NULL;
	g_patterns=NULL;
	g_supports=NULL;
}


// Dummy function - does nothing
void cuda_init(int myrank)
{

}


