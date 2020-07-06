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



boolean exit_thread=false;

void * allocate_loop(void *vargp){
	while(!exit_thread){

		int *size_c=(int *)malloc(1*sizeof(int));
		int *arr_c=(int *)malloc(ITERATIONS*sizeof(int));
		*size_c=ITERATIONS;

		for(int i=0;i<*size_c;i++){
			arr_c[i]=i;
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

	printf("testing allocation multithread...\n");

	for(unsigned i=0; i < N_TEST_THREAD; i++){
		pthread_create(&thread_test[i],NULL,allocate_loop,NULL);
	}

	getchar();

	exit_thread=true;
	for(unsigned i=0; i < N_TEST_THREAD; i++){
		pthread_join(thread_test[i], NULL);
	}

	MEMMGR_print_status();

	return 0;
}

//#include "memmgr.cpp"
