/*
 * Parallelization of the calculation of pi through the 
 * calculation of an integral using rectangle rule.
 *---------------------------------------------------------------------------------------------------
 * compile : mpicc -Wall -O3 -o pi pi.c -lm
 *
 *---------------------------------------------------------------------------------------------------
 * author : Josselin Lefevre 10/2020
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mpi.h"

/*
 *	Returns the value of the integral calculated over 
 *	the interval allocated to a node using n_subd rectangles.
 */
double rect_int(int n_subd, int rank, int size){
	unsigned int i;
    double x, val = 0.0;
    double step = 1.0/(size*n_subd);
    
    for(i=0; i<n_subd; ++i){
    	//compute x
       	x = i*step+(step/2.0)+step*rank*n_subd;
    	//compute f(x)
    	val += (1.0/(1.0+pow(x,2)))*step;
    }

    return val;
}

int main(int argc, char **argv){
    int rank, size, n_subd;
    double tmp_pi, pi, err, pi_gt=3.141592653589793238462643;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

	if(argc != 2){
		printf("Usage: %s <subdivision cnt>\n", argv[0]);
		exit(-1);
	}

    if(rank==0) n_subd = atoi(argv[1]);

    MPI_Bcast(&n_subd, 1, MPI_INT, 0, MPI_COMM_WORLD);
    tmp_pi = rect_int(n_subd, rank, size);
    MPI_Reduce(&tmp_pi, &pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if(rank==0){
        pi*=4.0;
        err=fabs(pi_gt-pi);
        printf("Pi~=%lf, err=%lf\n", pi, err);
    }
    MPI_Finalize();

    return 0;
}
