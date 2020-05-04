all: compsup.cu apriori.cpp
	mpic++ apriori.cpp -c -o apriori.o
	nvcc -arch=sm_70 compsup.cu -c -o compsup.o
	mpic++ apriori.o compsup.o -o apriori \-L/usr/local/cuda-10.1/lib64/ -lcuda -lcudart

read: reader/reader.cpp
	g++ reader/reader.cpp -o reader/reader

win: compsup.cu apriori.cpp
	nvcc apriori.cpp compsup.cu -o apriori.exe