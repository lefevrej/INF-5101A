#!/bin/bash

DDIR='../../data' #path to data
BDIR='./bin' #path to binary
ODIR='./benchmark' #output directory
MAT=(4 16 500 1000 5000 10000)
length=${#MAT[@]}

#test on sequential

#test on //

for (( i = 0; i < length; i++ )); do
	echo "------------- Benchmark for m${MAT[i]} -------------" > ${ODIR}/bench_m${MAT[i]}
	echo "" >> ${ODIR}/bench_m${MAT[i]}
	for p in 2 4 8 12 16 20; do
		if [ ${p} -lt ${MAT[i]} -o ${p} -eq ${MAT[i]} ]; then
			echo "---------------- nb-proc: ${p} -----------------" >> ${ODIR}/bench_m${MAT[i]}
			echo "" >> ${ODIR}/bench_m${MAT[i]}
  			echo "mpirun -np ${p} ${BDIR}/laplace ${MAT[i]} ${DDIR}/m${MAT[i]}"
  			mpirun -np ${p} ${BDIR}/laplace ${MAT[i]} ${DDIR}/m${MAT[i]} >> ${ODIR}/bench_m${MAT[i]}
			echo "" >> ${ODIR}/bench_m${MAT[i]}
		fi
	done
done
