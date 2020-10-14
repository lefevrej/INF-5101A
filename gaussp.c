
/*  tokenring example using PVM 3.4 
    - uses sibling() to determine the nb of spawned tasks (xpvm and pvm> ok)
    - uses group for token ring communication
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <string.h>
#include "pvm3.h"

#define GRPNAME "gauss"

//ok
void matrix_pload ( char *nom, double *tab, int N, int me, int *tids, int nproc) {
	FILE *f;
 	unsigned int i,j;	
	double val, tmp_tab[N];
	int stride = 1;
	int msgtag = 1;
	//printf("matrix_pload: P%d\n", me);
	fflush(0);

	if(me==0){
		if((f = fopen (nom, "r")) == NULL) { perror ("matrix_pload : fopen "); }
		for (i=0; i<N; i++) {
    		for (j=0; j<N; j++) {
      			fscanf(f, "%lf", &tmp_tab[j]);
    		}    		
			if(i%nproc==me){
				memcpy(&tab[(i/nproc)*N], tmp_tab, N*sizeof(double));
			} else {
				//send message to proc i%p
				pvm_initsend( PvmDataDefault );
				pvm_pkdouble(tmp_tab, N, stride); 
				//printf("matrix_pload: Send to P%d line n°%d\n", i%nproc, i);
				//fflush(0);
				pvm_send( (tids[i%nproc]), msgtag );
			}
  		}
  		fclose(f);
	}else{
		//printf("matrix_pload: P%d waiting for 0\n", me);
		fflush(0);	
		for(i=0;i<N/nproc;++i){			
			if(pvm_recv(-1, -1)<0)
				pvm_perror("matrix_pload: Bad recv\n");
			pvm_upkdouble(&tab[i*N], N, stride);
			//printf("P%d have received line n°%d from 0\n", me, i);
		}			
	}	
	//printf("P%d (%x) loading lines done\n", me, pvm_mytid());
	//fflush(0);
}

//issue
void matrix_psave(char nom[], double *tab, int N, int me, int *tids, int nproc) {
	FILE *f;
	int i,j;
	double *tmp_tab;
	int stride = 1;
	int msgtag = 1;  
  
	//printf("matrix_psave: P%d\n", me);
	
	if(me==0)
		if((f = fopen (nom, "w+")) == NULL){ perror ("matrix_save : fopen "); } 
  	for (i=0; i<N; i++) {
  		if(me==0){			
  			if(i%nproc==me){
  				tmp_tab = &tab[(i/nproc)*N];
  			} else {
  				//printf("Waiting line n°%d from P%d\n",i,i%nproc);
  				fflush(0);
	  			if(pvm_recv(tids[i%nproc],-1)<0)
					pvm_perror("matrix_psave: Bad recv\n");
					
	  			pvm_upkdouble(tmp_tab, N, stride);	 
  			}
			for (j=0; j<N; j++) {
	  			fprintf(f,"%8.2f ",tmp_tab[j]);
			}
			fprintf(f, "\n");
			
			//close file
	  		if(i==N-1){
	  			fclose(f);
	  			printf("==> Close file\n");
	  			fflush(0);
	  		}	  			
  		}else{
  			if(i%nproc==me){
	  			//printf("Sending from P%d to P0 (line n°%d)\n", me, i);
	  			//fflush(0);
				pvm_initsend( PvmDataDefault );
				pvm_pkdouble(&tab[(i/nproc)*N], N, stride); 
				pvm_send((tids[0]), msgtag);
			}
		}	
	} 
	//printf("P%d (%x) saving result done\n", me, pvm_mytid());
}

void gaussp( double *tab, double *l_pivot, int N, int k, int me, int di, int nproc){
	int i,j;
	double pivot;
	
	if (fabs(l_pivot[k]) <= 1.0e-11 ) { //check in order to avoid /0
	  printf("ATTENTION: pivot %d presque nul: %g\n", k, *(tab+k+k*N) );
	  fflush(0);
	  exit (-1);
	}
	for ( i=di; i<N/nproc; i++ ){ //update lines (k+1) to (n-1)
	  pivot = - tab[i*N+k] / l_pivot[k];
	  for ( j=k+1; j<N; j++ ){ //update elts (k) - (N-1) of line i 
		tab[i*N+j] = tab[i*N+j] + pivot * l_pivot[j];
	  }
	  tab[i*N+k] = 0.0;
	}	
}

void dowork(char* nom, int N, int me, int *tids, int nproc){
	unsigned int i, j, k, di=0, ip=0, stride=1, msg=1;
	double *tab, *l_pivot; 
	FILE *f;
	struct timeval tv1, tv2;	/* for timing */
	int duree1, duree2;
	
	printf("dowork: P%d\n", me);
	fflush(0);
	if ( (tab=malloc(N*N/nproc*sizeof(double))) == NULL ) {
		printf ("Cant malloc %ld bytes\n", N*N/nproc*sizeof(double));
		fflush(0);
		exit (-1);
	}
	if ( (l_pivot=malloc(N*sizeof(double))) == NULL ) {
		printf ("Cant malloc %ld bytes\n", N*sizeof(double));
		fflush(0);
		exit (-1);
	}
	
	gettimeofday( &tv1, (struct timezone*)0 );
	matrix_pload (nom , tab, N, me, tids, nproc);
	gettimeofday( &tv2, (struct timezone*)0 );
	duree1 = (tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec;

	gettimeofday( &tv1, (struct timezone*)0 );
	for(k=0;k<N-1;++k){
		if(k%nproc==me){
			pvm_initsend(PvmDataDefault);
			pvm_pkdouble(&tab[(k/nproc)*N], N, stride); 
			if(pvm_bcast(GRPNAME, msg)<0)
				pvm_perror("dowork: broadcast failed\n");
			//l_pivot=&tab[(k/nproc)*N]; ==> issue
			memcpy(l_pivot, &tab[(k/nproc)*N], N*sizeof(double)); 
			di++;
		} else {
			//printf("P%d waiting for line n°%d from P%d\n", me, k, k%nproc);
			fflush(0);
			if(pvm_recv(tids[k%nproc], -1)<0)
				pvm_perror("dowork: Bad recv\n");
			pvm_upkdouble(l_pivot, N, stride);
			//printf("P%d, line n°%d received form P%d\n", me, k, k%nproc);
			fflush(0);
		}		
		gaussp(tab, l_pivot, N, k, me, di, nproc);
	}
	gettimeofday( &tv2, (struct timezone*)0 );
	duree2 = (tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec;
	
	if(me==0){
		sprintf ( nom+strlen(nom), ".result" );
		printf("Results filename: %s\n", nom);
		printf("loading time: %10.8f sec.\n", duree1/1000000.0 );		
		printf ("computation time: %10.8f sec.\n", duree2/1000000.0 );
		fflush(0);
	}	
	//matrix_psave (nom , tab, N, me, tids, nproc);
	free(tab);
	free(l_pivot);
} 

int main(int argc, char ** argv) {
    int NPROC = 8;		/* default nb of proc */
    int mytid;                  /* my task id */
    int *tids;                  /* array of task id */
    int me;                     /* my process number */
    int i, N;
	char nom[255];
	
	//have to make sure that the number of proc is < to <matrix size> 
	//and is even
	
	if (argc != 3){
		char error[60];
		sprintf( error, "Usage: %s <matrix size> <matrix name>\n", argv[0] );
    	pvm_perror(error);
    	exit (-1);
	} 
	
    N = atoi ( argv[1] );
  	strcpy(nom, argv[2]);

    /* enroll in pvm */
    mytid = pvm_mytid();

    /* determine the size of my sibling list */
    NPROC = pvm_siblings(&tids); 
    /* WARNING: tids are in order of spawning, which is different from
       the task index JOINING the group */
       
	if(N%NPROC){
		N=N-(N%NPROC);
		printf("Number of processor incompatible with the number of line. New number of lines: %d\n",N-(N%NPROC));
	}

    me = pvm_joingroup( GRPNAME ); /* me: task index in the group */
    pvm_barrier( GRPNAME, NPROC );
    pvm_freezegroup ( GRPNAME, NPROC );
    for ( i = 0; i < NPROC; i++) tids[i] = pvm_gettid ( GRPNAME, i); 

	/*--------------------------------------------------------------------*/
	/*             all the tasks are equivalent at that point             */
	
	printf("main: %d\n", me);
	fflush(0);
	
    dowork( nom, N, me, tids, NPROC );

	pvm_lvgroup( GRPNAME );
	pvm_exit();
	printf("P%d -> done\n", me);
	return 0;
 }
