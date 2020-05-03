#ifndef __COMPSUP_H
#define __COMPSUP_H

void cuda_init(int myrank);

class Compute{
	char* g_patterns;
	char* g_dataset;
	unsigned int* g_supports;

	unsigned threadsCount; // number of threads

	size_t g_trans_len;
	size_t g_num_patterns;
	size_t g_num_data;

public:
	Compute(size_t trans_len, unsigned int num_threads):
					threadsCount(num_threads),g_trans_len(trans_len),
					g_patterns(NULL),g_dataset(NULL),g_supports(NULL){}
	void set_data(char*  dataset, size_t num_data);
	void allocate_data(size_t num_data);
	inline char* get_data_addr();
	void set_patterns(char* patterns, size_t num_patterns);
	void compute_support();
	void get_supports(unsigned int* supports);
	void free_all();
};

#endif