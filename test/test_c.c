#include <stdio.h>
#include <stdlib.h>

#ifdef __WIN32__
#include <synchapi.h>

#endif

#include <pthread.h>
#include <time.h>

#ifdef __GNUC__
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#endif


#include "memmgr.h"

#define N_TEST_THREAD 100
#define ITERATIONS	10000



int exit_thread=0;

void * allocate_loop(void *vargp){
	while(!exit_thread){

		int *size_c=(int *)malloc(1*sizeof(int));
		int *arr_c=(int *)malloc(ITERATIONS*sizeof(int));
		*size_c=ITERATIONS;

		for(int i=0;i<*size_c;i++){
			arr_c[i]=i;
		}

		if(rand()%2==0){
			arr_c=(int *)realloc(arr_c,(ITERATIONS>>1)*sizeof(int));
			for(int i=0;i<(ITERATIONS>>1);i++){
				arr_c[i]=(ITERATIONS>>1)-1;
			}

		}

		free(arr_c);
		free(size_c);


		int time_ms=rand()%10+10;
#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32) || defined(__WINDOWS__) || defined(__TOS_WIN__)
		Sleep( time_ms );
#else
		usleep( time_ms * 1000 );
#endif

	}

	return NULL;
}


int main(int argc, char *argv[]){

	srand(time(NULL));
	pthread_t thread_test[N_TEST_THREAD]={0};// (allocate_loop);

	printf("testing allocation multithread C\n");

	for(int i=0; i < N_TEST_THREAD; i++){
		pthread_create(&thread_test[i],NULL,allocate_loop,NULL);
	}

	printf("Press any key to stop...\n");

	getchar();

	printf("exiting...\n");

	exit_thread=1;
	for(int i=0; i < N_TEST_THREAD; i++){
		pthread_join(thread_test[i], NULL);
	}

	return 0;
}

//#include "memmgr.cpp"
