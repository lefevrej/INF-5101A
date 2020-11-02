/*
 * Reading then communication of data to nodes. The original matrix is broken down into blocks of lines.
 * Each node gets N/size lines. We also want each node to have access to the first row of the
 * next node and the last row of the previous node. That's why we communicate overlap lines
 *---------------------------------------------------------------------------------------------------
 * compile : mpicc -Wall -O3 -o data_par data_par.c -lm
 *
 *---------------------------------------------------------------------------------------------------
 * author : Josselin Lefevre 10/2020
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mpi.h"

void matrix_pload(char* name, int N, int rank, int size, double *tab){
    MPI_Status status;
    FILE *f;
    unsigned int i, j;
    int block_h = N/size;
    double *tmp_tab;
    
	if(rank==0){
		if((f = fopen (name, "r")) == NULL) { perror ("matrix_pload : fopen "); }

        if ((tmp_tab = malloc(N*block_h * sizeof(double))) == NULL){
            printf("Can't malloc tmp_tab\n");
            exit(-1);
        }

		for (i=0; i<size; i++) {
    		for (j=0; j<N*block_h; j++)
      			if(fscanf(f, "%lf", &tmp_tab[j])<0){
					perror("matrix_pload: bad read");
					exit(-1);
				}
            if(i==0)
                memcpy(&tab[N], tmp_tab, N*block_h*sizeof(double));
            else	
                MPI_Send(tmp_tab, N*block_h, MPI_DOUBLE, i, 99, MPI_COMM_WORLD);
  		}
  		fclose(f);
        free(tmp_tab);
    }
    if(rank!=0)
        MPI_Recv(&tab[N], N*block_h, MPI_DOUBLE, 0, 99, MPI_COMM_WORLD, &status);	
}

void matrix_psave(char* name, int N, int rank, int size, double *tab) {
	FILE *f;
	int i,j; 
	double *tmp_tab;
    MPI_Status status;
	
	if(rank==0)
		if((f = fopen (name, "w+")) == NULL){ perror("matrix_psave : fopen "); } 
  	for (i=0; i<size; i++) {
  		if(rank==0){			
  			if(i==0){
  				tmp_tab = &tab[N];
  			} else {
  				fflush(0);
                MPI_Recv(tmp_tab, (N/size)*N, MPI_DOUBLE, i, 99, MPI_COMM_WORLD, &status);
  			}
			for (j=0; j<(N/size)*N; j++) {
                if(j%N==0 && i+j!=0) fprintf(f, "\n");
	  			fprintf(f,"%8.2f ",tmp_tab[j]);
			}
			
			//close file
	  		if(i==N-1) fclose(f);	
  		}else{
  			if(rank==i)
                MPI_Send(&tab[N], (N/size)*N, MPI_DOUBLE, 0, 99, MPI_COMM_WORLD);
		}	
	} 
}

/**
 * Sending of overlap lines. You have to pay attention to the order 
 * of "send" and "receive" to avoid deadlocks.
 */
void send_overlap(int N, int rank, int size, double *tab){
    MPI_Status status;

    if(size==1) return;
    if(rank<size-1){
        //send last line to next processor
        MPI_Send(&tab[N*(N/size)], N, MPI_DOUBLE, rank+1, 99, MPI_COMM_WORLD);
        //receive first line of next processor
        MPI_Recv(&tab[(N)*(N/size+1)], N, MPI_DOUBLE, rank+1, 99, MPI_COMM_WORLD, &status);
    }
    if(rank>0){
        //receive last line from previous processor
        MPI_Recv(tab, N, MPI_DOUBLE, rank-1, 99, MPI_COMM_WORLD, &status);
        //send first line to the previous processor
        MPI_Send(&tab[N], N, MPI_DOUBLE, rank-1, 99, MPI_COMM_WORLD);
    }
}

void print_matrix(int rank, int rs, int cs, double *tab){
    printf("-----[P%d]-----\n", rank);
    for (int i=0; i<cs; i++) {
        for (int j=0; j<rs; j++) {
            printf("%5.2f ", tab[i*rs+j]);
        }    	
        printf("\n");
    }
    printf("-------------\n");
}

int main(int argc, char **argv){
    int rank, size, N, tmp;
    char name[255];
    double *tab;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(argc != 3){
		printf("Usage: %s <N> <filename>\n", argv[0]);
		exit(-1);
	}

    N = atoi(argv[1]);
    strcpy(name, argv[2]);

    if ((tab = malloc((N * (N/size+2)) * sizeof(double))) == NULL)
    {
        printf("Can't malloc blabla\n");
        exit(-1);
    }

    matrix_pload(name, N, rank, size, tab);
    send_overlap(N, rank, size, tab);
    //print matrix for each node properly
    if(size==1) print_matrix(rank, N, N/size+2, tab);
    else{
        if(rank==0){
            print_matrix(rank, N, N/size+2, tab);
            fflush(0);
            MPI_Send(&size, 1, MPI_INT, rank+1, 99, MPI_COMM_WORLD);
        } else {
            MPI_Recv(&tmp, 1, MPI_INT, rank-1, 99, MPI_COMM_WORLD, &status);
            print_matrix(rank, N, N/size+2, tab);
            fflush(0);
            if(rank<size-1)
                MPI_Send(&size, 1, MPI_INT, rank+1, 99, MPI_COMM_WORLD);
        }
    }

    //save lines, output file should be identical to input
    sprintf(name+strlen(name), ".result" );
    matrix_psave(name, N, rank, size, tab);

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    free(tab);

    return 0;
}