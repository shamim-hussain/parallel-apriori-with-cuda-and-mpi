all: compsup.cu test_apriori.cpp
	mpic++ test_apriori.cpp -c -o test_apriori.o
	nvcc -arch=sm_70 compsup.cu -c -o compsup.o
	mpic++ test_apriori.o compsup.o -o test_apriori \-L/usr/local/cuda-10.1/lib64/ -lcuda -lcudart

read: read_pat_sup.cpp
	g++ read_pat_sup.cpp -o read_pat_sup

win: compsup.cu test_apriori.cpp
	nvcc test_apriori.cpp compsup.cu -o test_apriori.exe