#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mpi.h"

int matrix_pload(char* name, int N, int rank, int size, double *tab)
{
    printf("matrix_pload: P%d\n", rank);
    MPI_Status status;
    FILE *f;
    unsigned int i, j;
    int stride = 1;
    int msgtag = 1;
    int block_h = N/size;
    double val, *tmp_tab;
    
	if(rank==0){
		if((f = fopen (name, "r")) == NULL) { perror ("matrix_pload : fopen "); }

        if ((tmp_tab = malloc(N*block_h * sizeof(double))) == NULL)
        {
            printf("Can't malloc tmp_tab\n");
            exit(-1);
        }

		for (i=0; i<size; i++) {
    		for (j=0; j<N*block_h; j++) {
      			fscanf(f, "%lf", &tmp_tab[j]);
                //printf("%5.2f ", tmp_tab[j]);
    		}   
            printf("\n");
            if(i==0){
                memcpy(&tab[N], tmp_tab, N*block_h*sizeof(double));
                //for (j=0; j<N; j++) {
                    //printf("%5.2f ", tab[N*(N/size)+j]);
    		    //}      
            //printf("\n");
            } else {
                printf("Send to P%d\n", i); 		
                MPI_Send(tmp_tab, N*block_h, MPI_DOUBLE, i, 99, MPI_COMM_WORLD);
                printf("Balblou\n");
            }
  		}
  		fclose(f);
        free(tmp_tab);
    }
    if(rank!=0){
        printf("P%d wait...\n", rank); 
        MPI_Recv(&tab[N], N*block_h, MPI_DOUBLE, 0, 99, MPI_COMM_WORLD, &status);		
        printf("P%d have received\n", rank); 
        fflush(0);
    }
    if(rank<size-1){
        printf("P%d if 1\n", rank); 
        //envoyer derniere ligne au processeur suivant
        MPI_Send(&tab[N*(N/size)], N, MPI_DOUBLE, rank+1, 99, MPI_COMM_WORLD);
        //recevoir premiere ligne du processuer suivant
        MPI_Recv(&tab[(N)*(N/size+1)], N, MPI_DOUBLE, rank+1, 99, MPI_COMM_WORLD, &status);
    }
    if(rank>0){
        printf("P%d if 2\n", rank); 
        //envoyer premiere ligne au processeur precedent
        MPI_Send(&tab[N], N, MPI_DOUBLE, rank-1, 99, MPI_COMM_WORLD);
        //recevoir derniere ligne du processeur precedent
        MPI_Recv(tab, N, MPI_DOUBLE, rank-1, 99, MPI_COMM_WORLD, &status);
    }
}

int main(int argc, char **argv)
{
    int rank, size = -1, N;
    char name[255];
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 3)
    {
        printf("Usage\n");
        printf("  Arg1 = number of subdivisions / unkowns\n");
        fflush(0);
        exit(-1);
    }
    printf("size=%d\n", size);

    N = atoi(argv[1]);
    strcpy(name, argv[2]);

    double *tab;
    if ((tab = malloc((N * (N/size+2)) * sizeof(double))) == NULL)
    {
        printf("Can't malloc blabla\n");
        exit(-1);
    }

    matrix_pload(name, N, rank, size, tab);

    if(rank==1){
        printf("Check P%d data: \n", rank);
        for (int i=0; i<(N / size+2); i++) {
            for (int j=0; j<N; j++) {
                printf("%5.2f ", tab[i*N+j]);
            }    	
            printf("\n");
        }
    }

    MPI_Finalize();
    free(tab);

    return 0;
}