#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mpi.h"

void matrix_pload(char* name, int N, int rank, int size, double *tab){
    printf("matrix_pload: P%d\n", rank);
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
      			fscanf(f, "%lf", &tmp_tab[j]);
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
    
    if(rank<size-1){
        //send last line to next processor
        MPI_Send(&tab[N*(N/size)], N, MPI_DOUBLE, rank+1, 99, MPI_COMM_WORLD);
        //receive first line of next processor
        MPI_Recv(&tab[(N)*(N/size+1)], N, MPI_DOUBLE, rank+1, 99, MPI_COMM_WORLD, &status);
    }
    if(rank>0){
        //send first line to the previous processor
        MPI_Send(&tab[N], N, MPI_DOUBLE, rank-1, 99, MPI_COMM_WORLD);
        //receive last line from previous processor
        MPI_Recv(tab, N, MPI_DOUBLE, rank-1, 99, MPI_COMM_WORLD, &status);
    }
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
                if(j%N==0 && i+j!=0)
			        fprintf(f, "\n");
	  			fprintf(f,"%8.2f ",tmp_tab[j]);
			}
			
			//close file
	  		if(i==N-1){
	  			fclose(f);
	  		}	  			
  		}else{
  			if(rank==i){
                MPI_Send(&tab[N], (N/size)*N, MPI_DOUBLE, 0, 99, MPI_COMM_WORLD);
			}
		}	
	} 
}

void print_matrix(int rank, int size, int N, double *tab){
    printf("-----[P%d]-----\n", rank);
    for (int i=0; i<(N / size+2); i++) {
        for (int j=0; j<N; j++) {
            printf("%5.2f ", tab[i*N+j]);
        }    	
        printf("\n");
    }
    printf("-------------\n");
}

int main(int argc, char **argv){
    int rank, size, N;
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
    printf("size=%d\n", size);

    N = atoi(argv[1]);
    strcpy(name, argv[2]);

    if ((tab = malloc((N * (N/size+2)) * sizeof(double))) == NULL)
    {
        printf("Can't malloc blabla\n");
        exit(-1);
    }

    matrix_pload(name, N, rank, size, tab);
    sprintf(name+strlen(name), ".result" );
    matrix_psave(name, N, rank, size, tab);

    print_matrix(rank, size, N, tab);
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    free(tab);

    return 0;
}