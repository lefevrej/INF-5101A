#!/bin/bash

if [[ $# -eq 0 ]]; then
	echo "Usage: $0 <convergence_value>"
	exit 1
fi


DDIR=../../data #path to data
BDIR=./bin #path to binaries
ODIR=./benchmark_$1 #output directory
MAT=(4 16 500 1000 5000 10000)
length=${#MAT[@]}

mkdir -p ${ODIR} 
#test on sequential

#test on //

echo "Benchmarking with convergence value as $1."
echo

for (( i = 0; i < length; i++ )); do
	echo "------------- Benchmark for m${MAT[i]} -------------" > ${ODIR}/bench_m${MAT[i]}
	echo  >> ${ODIR}/bench_m${MAT[i]}
	for p in 2 4 8 12 16 20; do
		if [ ${p} -lt ${MAT[i]} -o ${p} -eq ${MAT[i]} ]; then
			echo "---------------- nb-proc: ${p} -----------------" >> ${ODIR}/bench_m${MAT[i]}
			echo  >> ${ODIR}/bench_m${MAT[i]}
			echo "mpirun -np ${p} ${BDIR}/laplace ${MAT[i]} ${DDIR}/m${MAT[i]} $1"
			mpirun -np ${p} ${BDIR}/laplace ${MAT[i]} ${DDIR}/m${MAT[i]} $1 >> ${ODIR}/bench_m${MAT[i]}
			echo  >> ${ODIR}/bench_m${MAT[i]}
		fi
	done
done

echo
echo "Done."
echo "Results available in: ${ODIR}"
