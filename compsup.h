#ifndef __COMPSUP_H
#define __COMPSUP_H


class Compute{
	char* g_patterns=NULL;
	char* g_dataset=NULL;
	unsigned int* g_supports=NULL;

	unsigned threadsCount; // number of threads

	size_t g_trans_len;
	size_t g_num_patterns=0;
	size_t g_num_data=0;

public:
	Compute(size_t trans_len, unsigned int num_threads):
					threadsCount(num_threads),g_trans_len(trans_len){}
	void set_data(char*  dataset, size_t num_data);
	void set_patterns(char* patterns, size_t num_patterns);
	void compute_support();
	void get_supports(unsigned int* supports);
	void free_all();
};

#endif