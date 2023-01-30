#include "memmgr.c"

#undef	new
#undef	delete

#define DEFAULT_CPP_ALIGNMENT	alignof(std::max_align_t)

//#define DEFINE_PUSH_FILE_LINE_TYPE(__type__)
//---------
// DELETE


static char registered_file_new[MEMMGR_MAX_STACK_FILE_LINE][MEMMGR_MAX_FILENAME_LENGTH]={0};
static int 	registered_line_new[MEMMGR_MAX_STACK_FILE_LINE]={-1};
static int 	n_registered_file_line_new=0;
static 		pthread_mutex_t mutex_file_line_new;

bool	MEMMGR_push_file_line_new(const  char  *absolute_filename,   int   line)
{
	pthread_mutex_lock(&mutex_file_line_new);
	if(n_registered_file_line_new < MEMMGR_MAX_STACK_FILE_LINE)\
	{\
		MEMMGR_get_filename(registered_file_new[n_registered_file_line_new],absolute_filename);
		registered_line_new[n_registered_file_line_new]=line;\
		n_registered_file_line_new++;\
	}\
	else\
	{\
		MEMMGR_LOG_INFOF(__FILE__\
			,__LINE__\
			,"reached max stacked files new");\
	}\
	pthread_mutex_unlock(&mutex_file_line_new);
	return true;\
}

//---------
// DELETE

static char registered_file_delete[MEMMGR_MAX_STACK_FILE_LINE][MEMMGR_MAX_FILENAME_LENGTH]={0};
static int 	registered_line_delete[MEMMGR_MAX_STACK_FILE_LINE]={-1};
static int 	n_registered_file_line_delete=0;
static 		pthread_mutex_t mutex_file_line_delete;

bool	MEMMGR_push_file_line_delete(const  char  *absolute_filename,   int   line)\
{
	pthread_mutex_lock(&mutex_file_line_delete);\
	if(n_registered_file_line_delete < MEMMGR_MAX_STACK_FILE_LINE)
	{
		MEMMGR_get_filename(registered_file_delete[n_registered_file_line_delete],absolute_filename);
		registered_line_delete[n_registered_file_line_delete]=line;\
		n_registered_file_line_delete++;
	}
	else
	{
		MEMMGR_LOG_INFOF(__FILE__
			,__LINE__\
			,"reached max stacked files delete."
			" Check that if somewhere is not deallocating a NULL pointer. NULL pointers NEVER calls override delete so it will increments the references on each case");\
	}
	pthread_mutex_unlock(&mutex_file_line_delete);
	return true;\
}

//DEFINE_PUSH_FILE_LINE_TYPE(_new)
//DEFINE_PUSH_FILE_LINE_TYPE(_delete)

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


	if((pointer=MEMMGR_malloc_alignment(_size,source_file,source_line,DEFAULT_CPP_ALIGNMENT))==NULL){
		if(_size == 0){
			return NULL;
		}
		throw std::bad_alloc();
	}

	PointerPreHeapInfo  *pre_head  =  GET_PREHEADER(pointer,DEFAULT_CPP_ALIGNMENT);
	pre_head->type_allocator  =  NEW_ALLOCATOR;


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

	if((pointer  =  MEMMGR_malloc_alignment(_size,source_file, source_line,DEFAULT_CPP_ALIGNMENT))==NULL){
		// 0 bytes allocation is allowed ?
		if(_size == 0){
			return NULL;
		}

		throw std::bad_alloc();
	}

	PointerPreHeapInfo  *pre_head  =  GET_PREHEADER(pointer,DEFAULT_CPP_ALIGNMENT);
	pre_head->type_allocator  =  NEW_WITH_BRACETS_ALLOCATOR;


	return  pointer;

}
//--------------------------------------------------------------------------------------------
void  operator  delete(void  *pointer, size_t _size) _NO_EXCEPT_TRUE{
	(((void)pointer),(void)_size);
	std::runtime_error("operator delete(void  *pointer, size_t _size) not implemented");
}

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

	preheap_allocat  =  GET_PREHEADER(pointer,DEFAULT_CPP_ALIGNMENT);
	postheap_allocat  =  GET_POSTHEADER(pointer,DEFAULT_CPP_ALIGNMENT);

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

	MEMMGR_free(pointer,  source_file,  source_line,DEFAULT_CPP_ALIGNMENT);

}
//--------------------------------------------------------------------------------------------
void  operator  delete[](void  *pointer, size_t _size) _NO_EXCEPT_TRUE{
	(((void)pointer),(void)_size);
	std::runtime_error("operator delete(void  *pointer, size_t _size) not implemented");
}

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

	preheap_allocat  =  GET_PREHEADER(pointer,DEFAULT_CPP_ALIGNMENT);
	postheap_allocat  =  GET_POSTHEADER(pointer,DEFAULT_CPP_ALIGNMENT);

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

	MEMMGR_free(pointer,  source_file,  source_line,DEFAULT_CPP_ALIGNMENT);

}

