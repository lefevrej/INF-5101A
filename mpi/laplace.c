#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include "mpi.h"

#define CONV 0.0001

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
      			if(fscanf(f, "%lf", &tmp_tab[j])!=1) perror("matrix_pload: bad read");
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
                if(j%N==0&& i+j!=0) fprintf(f, "\n");
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

void send_overlap(int N, int rank, int size, double *tab){
    MPI_Status status;

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

double laplace(int rank, int size, int N, double *tab, double *tmp_tab){
    double err=0.0;
    //offset for first and last line
    int offset_t = rank==0 ? 1 : 0;
    int offset_b = rank==size-1 ? 1 : 0;

    for(int i=1+offset_t; i<N/size+1-offset_b; ++i)
        for(int j=1; j<N-1; ++j){
            tmp_tab[(i-1)*N+j]=0.25*(tab[(i+1)*N+j]+tab[(i-1)*N+j]
            +tab[i*N+(j+1)]+tab[i*N+(j-1)]);
            err+=powf(tmp_tab[(i-1)*N+j]-tab[i*N+j], 2);
        }

    
    for(int i=1+offset_t; i<N/size+1-offset_b; ++i)
        for(int j=1; j<N-1; ++j)
            tab[i*N+j] = tmp_tab[(i-1)*N+j];
    return err;
}

void do_work(int rank, int size, int N, char *name){
    double *tab, *tmp_tab, err,  g_err, tmp_gerr;
    struct timeval tv1, tv2;	/* for timing */
	int duree1, duree2;

    if( (tab = malloc((N * (N/size+2)) * sizeof(double))) == NULL ){
        printf("Can't malloc blabla\n");
        exit(-1);
    }
    if( (tmp_tab = malloc((N * (N/size)) * sizeof(double))) == NULL ){
        printf("Can't malloc blabla\n");
        exit(-1);
    }

    if(rank==0 || rank==size-1){
        int shift = rank==0 ? 0 : (N/size+1)*N;
        for(int i=0; i<N; ++i)
            tab[shift+i]=-1.0;
    }

    gettimeofday( &tv1, (struct timezone*)0 );
    matrix_pload(name, N, rank, size, tab);
    gettimeofday( &tv2, (struct timezone*)0 );
	duree1 = (tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec;
    
    gettimeofday( &tv1, (struct timezone*)0 );
    do{
        send_overlap( N, rank, size, tab);

        tmp_gerr=g_err;
        err=laplace(rank, size, N, tab, tmp_tab);
        MPI_Allreduce(&err, &g_err, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
        g_err=sqrt(g_err);
    }while( fabs(g_err-tmp_gerr)>CONV );
    gettimeofday( &tv2, (struct timezone*)0 );
	duree2 = (tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec;

    if(rank==0){
        sprintf(name+strlen(name), ".result" );
        printf("loading time: %10.8f sec.\n", duree1/1000000.0 );		
		printf ("computation time: %10.8f sec.\n", duree2/1000000.0 );
        fflush(0);
    }
    matrix_psave(name, N, rank, size, tab);

    free(tab);
    free(tmp_tab);
}

int main(int argc, char **argv){
    int rank, size, N;
    char name[255];
    
    if(argc != 3){
		printf("Usage: %s <N> <filename>\n", argv[0]);
		exit(-1);
	}

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    N = atoi(argv[1]);
    strcpy(name, argv[2]);

    if(N%size!=0){
		N=N-(N%size);
        if(rank==0)
		    printf("Number of processor incompatible with the number of line. New number of lines: %d\n",N-(N%size));
	}

    do_work(rank, size, N, name);
    
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();

    return 0;
}