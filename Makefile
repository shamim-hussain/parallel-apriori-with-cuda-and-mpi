all: compsup.cu apriori.cpp
	mpic++ apriori.cpp -c -o apriori.o
	nvcc -arch=sm_70 compsup.cu -c -o compsup.o
	mpic++ apriori.o compsup.o -o apriori \-L/usr/local/cuda-10.1/lib64/ -lcuda -lcudart

reader: reader/reader.cpp
	g++ reader/reader.cpp -o reader/reader

win: compsup.cu apriori.cpp
	nvcc apriori.cpp compsup.cu -o apriori.exe

run: slurmSpectrum.sh
	sbatch -N $(n) --partition=dcs --ntasks-per-node=$(t) --gres=gpu:$(g) -t 30 ./slurmSpectrum.sh

read: reader/reader
	reader/reader $(tl)
rd20: reader/reader
	reader/reader | tail -n 20

rmout: patterns.dat supports.dat
	rm patterns.dat
	rm supports.dat

rmslurm: 
	rm -r slurm-*.out