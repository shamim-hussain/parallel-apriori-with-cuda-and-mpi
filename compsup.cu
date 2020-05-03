#include<stdio.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include "compsup.h"


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


void Compute::set_data(char*  dataset, size_t num_data){
	g_num_data=num_data;
	if (g_dataset!=NULL) cudaFree(g_dataset);
	cudaMalloc(&g_dataset, g_num_data * g_trans_len * sizeof(char));
	cudaMemcpy(g_dataset, dataset, g_num_data * g_trans_len * sizeof(char),
					cudaMemcpyHostToDevice);
}

void Compute::allocate_data(size_t num_data){
	g_num_data=num_data;
	if (g_dataset!=NULL) cudaFree(g_dataset);
	cudaMallocManaged(&g_dataset, g_num_data * g_trans_len * sizeof(char));
}

char* Compute::get_data_addr(){
	return g_dataset;
}

void Compute::set_patterns(char* patterns, size_t num_patterns){
	g_num_patterns=num_patterns;
	if (g_patterns!=NULL)cudaFree(g_patterns);				
	cudaMalloc(&g_patterns, g_num_patterns * g_trans_len * sizeof(char)); 
	cudaMemcpy(g_patterns, patterns, g_num_patterns * g_trans_len * sizeof(char),
				cudaMemcpyHostToDevice);
}

void Compute::compute_support(){
		//threading info           
		size_t blocksCount = (g_num_patterns+threadsCount-1)/threadsCount;
		
		if (g_supports!=NULL) cudaFree(g_supports);
		cudaMallocManaged(&g_supports, g_num_patterns * sizeof(unsigned int));
		
		compute_support_kernel<<<blocksCount, threadsCount>>> (g_patterns, g_num_patterns,
																g_dataset, g_num_data, 
													 			g_trans_len, g_supports);
		
		cudaDeviceSynchronize();
}


void Compute::get_supports(unsigned int* supports){
	cudaMemcpy(supports, g_supports, g_num_patterns * sizeof(unsigned int),
				 cudaMemcpyDeviceToHost);
}

void Compute::free_all(){
	if (g_dataset!=NULL) cudaFree(g_dataset);
	if (g_patterns!=NULL)cudaFree(g_patterns);
	if (g_supports!=NULL) cudaFree(g_supports);
	g_dataset=NULL;
	g_patterns=NULL;
	g_supports=NULL;
}


// Function to initialize CUDA device
void cuda_init(int myrank)
{
    int cudaDeviceCount;
    cudaError_t cE;
    
    if( (cE = cudaGetDeviceCount( &cudaDeviceCount)) != cudaSuccess ){
        printf(" Unable to determine cuda device count, error is %d, count is %d\n",
                                                                cE, cudaDeviceCount );
        exit(-1);
    }

    if( (cE = cudaSetDevice( myrank % cudaDeviceCount )) != cudaSuccess ){
        printf(" Unable to have rank %d set to cuda device %d, error is %d \n",
        myrank, (myrank % cudaDeviceCount), cE);
        exit(-1);
    } 
}


