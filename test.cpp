#define __MEMMGR__

#include <thread>
#ifdef __GNUC__
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#endif


#include "memmgr.cpp"

#define N_TEST_THREAD 1
#define ITERATIONS	100


bool exit_thread=false;

void allocate_loop(){
	while(!exit_thread){

		int *arr_c=(int *)malloc(ITERATIONS*sizeof(int));
		int *size_c=(int *)malloc(1*sizeof(int));
		*size_c=ITERATIONS;

		int *arr_cpp=new int[ITERATIONS];
		int *size_cpp=new int;
		*size_cpp=ITERATIONS;

		for(int i=0;i<*size_cpp;i++){
			arr_cpp[i]=i;
		}

		for(int i=0;i<*size_c;i++){
			arr_c[i]=i;
		}


		delete [] arr_cpp;
		free(arr_c);


		int time_ms=rand()%10+10;
#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32) || defined(__WINDOWS__) || defined(__TOS_WIN__)
		Sleep( time_ms );
#else
		usleep( time_ms * 1000 );
#endif

	}
}


int main(int argc, char *argv[]){

	srand(time(NULL));
	std::thread *thread_test[N_TEST_THREAD]={NULL};// (allocate_loop);

	printf("testing allocation multithread...\n");

	for(unsigned i=0; i < N_TEST_THREAD; i++){
		thread_test[i]=new std::thread(allocate_loop);
	}





	// main thread allocate
	/*for(;;){

		if(getchar()){
			break;
		}
	}*/

	getchar();

	for(unsigned i=0; i < N_TEST_THREAD; i++){
		exit_thread=true;
		thread_test[i]->join();
	}

	MEMMGR_print_status();

	return 0;
}
