#ifndef __COMPSUP_H
#define __COMPSUP_H

void compute_support(char* patterns, size_t num_patterns, 
						char*  dataset, size_t num_data, 
						size_t trans_len, unsigned int* supports);

#endif