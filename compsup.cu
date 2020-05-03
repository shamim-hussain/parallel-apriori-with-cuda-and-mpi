#include<stdio.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include "compsup.h"

char* g_patterns=NULL;
char* g_dataset=NULL;
unsigned int* g_supports=NULL;

#define NUM_THREADS 256;


__global__  void compute_support_kernel(char* patterns, size_t num_patterns, 
										char*  dataset, size_t num_data,
										size_t trans_len, unsigned int* supports);

void hostToDeviceMemory(char* patterns, size_t num_patterns, 
						char*  dataset, size_t num_data,
						size_t trans_len, unsigned int* supports){
	
	//allocate data variables						
	cudaMallocManaged(&g_patterns, num_patterns * trans_len * sizeof(char)); 
	cudaMemcpy(g_patterns, patterns, num_patterns * trans_len * sizeof(char),
				cudaMemcpyHostToDevice);
	
	cudaMallocManaged(&g_dataset, num_data * trans_len * sizeof(char));
	cudaMemcpy(g_dataset, dataset, num_data * trans_len * sizeof(char),
				 cudaMemcpyHostToDevice);
	
	cudaMallocManaged(&g_supports, num_patterns * sizeof(unsigned int));	
}

void compute_support(char* patterns, size_t num_patterns, 
						char*  dataset, size_t num_data, 
						size_t trans_len, unsigned int* supports){
		
		//threading info                    
		unsigned threadsCount = NUM_THREADS; // number of threads
		size_t blocksCount = (num_patterns+threadsCount-1)/threadsCount;

		//memory copy from host to device
		hostToDeviceMemory(patterns, num_patterns, dataset, num_data, trans_len, supports);

		//kernel invoke
		compute_support_kernel<<<blocksCount, threadsCount>>> (g_patterns, num_patterns,
																g_dataset, num_data, 
													 			trans_len, g_supports);
		
		cudaDeviceSynchronize();
		cudaMemcpy(supports, g_supports, num_patterns * sizeof(unsigned int),
				 cudaMemcpyDeviceToHost);		
}

__global__  void compute_support_kernel(char* patterns, size_t num_patterns, 
						char*  dataset, size_t num_data,
						size_t trans_len, unsigned int* supports){
		
		unsigned int ind_x=blockIdx.x * blockDim.x + threadIdx.x;
		char* pat_j = patterns+ind_x*trans_len;
		char* dat_i;
		char* dat_end = dataset+num_data*trans_len;
		

		if (ind_x>=num_patterns) return;
		
		unsigned int sup_j = 0;
		unsigned int k;
		int not_subset;
		for (dat_i = dataset; dat_i<dat_end;dat_i+=trans_len){
			not_subset=0;
			for (k=0;k<trans_len;k++){
				not_subset = not_subset | (pat_j[k]&(~dat_i[k]));
			}
			sup_j =sup_j + !not_subset;
		}
		supports[ind_x]=sup_j;
}

// C version (for Neehal and Shoron)
void compute_support_n(char* patterns, size_t num_patterns, 
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