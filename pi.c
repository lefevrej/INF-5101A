#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mpi.h"

double rect_int(int n_subd, int rank){
    double val = 0.0;
    double step = 1.0/n_subd;

    double x = step*rank+(step/2.0);
    val = (1.0/(1.0+pow(x,2)))*step;
    //printf("P%d -> %f\n", rank,  val);
    return val;
}

int main(int argc, char **argv){
    int rank, size=-1;
    double tmp_pi, pi, err, rpi=3.141592653589793238462643;

    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if( argc < 2 ) {
        printf("Usage\n");
        printf("  Arg1 = number of subdivisions / unkowns\n");
        return -1;
    }

    size = atoi(argv[1]);

    MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    tmp_pi = rect_int(size,rank);

    MPI_Reduce(&tmp_pi, &pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if(rank==0){
        pi*=4.0;
        err=fabs(rpi-pi);
        printf("Pi=%f, err=%f\n", pi, err);
    }
    MPI_Finalize();

    return 0;
}