#ifndef __COMPSUP_H
#define __COMPSUP_H


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
	void set_patterns(char* patterns, size_t num_patterns);
	void compute_support();
	void get_supports(unsigned int* supports);
	void free_all();
};

#endif