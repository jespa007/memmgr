#include "memmgr.c"

#undef	new
#undef	delete

#define DEFINE_PUSH_FILE_LINE_TYPE(__type__) \
\
static char registered_file##__type__[MEMMGR_MAX_STACK_FILE_LINE][MEMMGR_MAX_FILENAME_LENGTH]={0};\
static int 	registered_line##__type__[MEMMGR_MAX_STACK_FILE_LINE]={-1};\
static int 	n_registered_file_line##__type__=0;\
static 		pthread_mutex_t mutex_file_line##__type__;\
\
bool	MEMMGR_push_file_line##__type__(const  char  *absolute_filename,   int   line)\
{\
	pthread_mutex_lock(&mutex_file_line##__type__);\
	if(n_registered_file_line##__type__ < MEMMGR_MAX_STACK_FILE_LINE)\
	{\
		MEMMGR_get_filename(registered_file##__type__[n_registered_file_line##__type__],absolute_filename);\
		registered_line##__type__[n_registered_file_line##__type__]=line;\
		n_registered_file_line##__type__++;\
	}\
	else\
	{\
		MEMMGR_LOG_INFOF(__FILE__\
			,__LINE__\
			,"reached max stacked files "#__type__"."\
			"If the issue is with delete or delete [] check that is not deallocating NULL pointer. NULL pointers NEVER calls override delete so it will increments the references on each case");\
	}\
	pthread_mutex_unlock(&mutex_file_line##__type__);\
	return true;\
}\

DEFINE_PUSH_FILE_LINE_TYPE(_new)
DEFINE_PUSH_FILE_LINE_TYPE(_delete)

void*  operator  new(size_t  _size) _THROW_BAD_ALLOC
{

	char source_file[MEMMGR_MAX_FILENAME_LENGTH]={"??"};
	int source_line=0;

	pthread_mutex_lock(&mutex_file_line_new);//.lock();


	if(n_registered_file_line_new > 0)
	{
		--n_registered_file_line_new;
		strcpy(source_file,registered_file_new[n_registered_file_line_new]);
		source_line = registered_line_new[n_registered_file_line_new];
	}

	//mutex_file_line.unlock();
	pthread_mutex_unlock(&mutex_file_line_new);


	void *pointer = NULL;


	if((pointer=MEMMGR_malloc(_size,source_file,source_line))!=NULL)
	{
		PointerPreHeapInfo  *pre_head  =  GET_PREHEADER(pointer);
		pre_head->type_allocator  =  NEW_ALLOCATOR;
	}

	return  pointer;
}
//--------------------------------------------------------------------------------------------
void*  operator  new[](size_t  _size) _THROW_BAD_ALLOC
{
	/*if(n_registered_file_line==0){
		return malloc(size);
	}*/
	char source_file[MEMMGR_MAX_FILENAME_LENGTH]={"??"};
	int source_line=0;

	if(n_registered_file_line_new > 0)
	{
		--n_registered_file_line_new;
		strcpy(source_file,registered_file_new[n_registered_file_line_new]);
		source_line = registered_line_new[n_registered_file_line_new];
	}

	//mutex_file_line.unlock();
	pthread_mutex_unlock(&mutex_file_line_new);


	void *pointer = NULL;

	if((pointer  =  MEMMGR_malloc(_size,source_file, source_line))!=NULL){
		PointerPreHeapInfo  *pre_head  =  GET_PREHEADER(pointer);
		pre_head->type_allocator  =  NEW_WITH_BRACETS_ALLOCATOR;
	}

	return  pointer;

}
//--------------------------------------------------------------------------------------------
void  operator  delete(void  *pointer) _NO_EXCEPT_TRUE
{
	PointerPreHeapInfo *preheap_allocat=NULL;
	PointerPostHeapInfo *postheap_allocat=NULL;

	//---------------------------------------------------
	// GET FILE/LINE
	pthread_mutex_lock(&mutex_file_line_delete);


	char source_file[MEMMGR_MAX_FILENAME_LENGTH]={"??"};
	int source_line=0;

	if(n_registered_file_line_delete > 0)
	{
		--n_registered_file_line_delete;
		strcpy(source_file,registered_file_delete[n_registered_file_line_delete]);
		source_line = registered_line_delete[n_registered_file_line_delete];
	}

	pthread_mutex_unlock(&mutex_file_line_delete);
	//mutex_file_line.unlock();

	if(pointer == NULL)
	{
		MEMMGR_LOG_WARNINGF(source_file,  source_line,"delete: Trying to deallocate NULL pointer!");
		return;
	}

	preheap_allocat  =  GET_PREHEADER(pointer);
	postheap_allocat  =  GET_POSTHEADER(pointer);

	if(preheap_allocat->pre_crc  !=  postheap_allocat->post_crc)
	{
		MEMMGR_LOG_ERRORF(source_file,source_line,"delete: Trying to deallocate a pointer with CRC error. Either is a corrupted pointer or not managed pointer!");
		return;
	}

	if(preheap_allocat->type_allocator  !=  NEW_ALLOCATOR)
	{
		MEMMGR_print_error_on_wrong_deallocate_method(source_file,  source_line,preheap_allocat->type_allocator);
		return;
	}

	MEMMGR_free(pointer,  source_file,  source_line);

}
//--------------------------------------------------------------------------------------------
void  operator  delete[](void  *pointer) _NO_EXCEPT_TRUE
{
	PointerPreHeapInfo *preheap_allocat=NULL;
	PointerPostHeapInfo *postheap_allocat=NULL;


	pthread_mutex_lock(&mutex_file_line_delete);

	char source_file[MEMMGR_MAX_FILENAME_LENGTH]={"??"};
	int source_line(0);

	if(n_registered_file_line_delete > 0)
	{
		--n_registered_file_line_delete;
		strcpy(source_file,registered_file_delete[n_registered_file_line_delete]);
		source_line = registered_line_delete[n_registered_file_line_delete];

	}

	pthread_mutex_unlock(&mutex_file_line_delete);


	if(pointer==NULL)
	{
		MEMMGR_LOG_WARNINGF(source_file,  source_line,"delete[]: Trying to deallocate NULL pointer");
		return;
	}

	preheap_allocat  =  GET_PREHEADER(pointer);
	postheap_allocat  =  GET_POSTHEADER(pointer);

	//  Check  headers...
	if(preheap_allocat->pre_crc  !=  postheap_allocat->post_crc)  //  crc  ok  :)
	{
		MEMMGR_LOG_ERRORF(source_file,  source_line,"delete[]: Trying to deallocate a pointer with CRC error. Either is a corrupted pointer or not managed pointer!");
		return;
	}

	if(preheap_allocat->type_allocator  !=  NEW_WITH_BRACETS_ALLOCATOR)
	{
		MEMMGR_print_error_on_wrong_deallocate_method(source_file,  source_line,preheap_allocat->type_allocator);
		return;
	}

	MEMMGR_free(pointer,  source_file,  source_line);

}

