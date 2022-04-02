#include "memmgr.c"

#undef	new
#undef	delete


static char registered_file[MAX_REGISTER_FILELINES][MEMMGR_MAX_FILENAME_LENGTH]={0};
static int 	registered_line[MAX_REGISTER_FILELINES]={-1};
static int 	n_registered_file_line=0;
static 		pthread_mutex_t mutex_file_line;

bool	MEMMGR_push_file_line(const  char  *absolute_filename,   int   line)
{

	pthread_mutex_lock(&mutex_file_line);//.lock();

	if(n_registered_file_line < MAX_REGISTER_FILELINES)
	{
		MEMMGR_get_filename(registered_file[n_registered_file_line],absolute_filename);
		registered_line[n_registered_file_line]=line;
		n_registered_file_line++;
	}
	else
	{
		LOG_LEVEL_INFOF(__FILE__,__LINE__,"reached max stacked files!");
		//return false;
	}

	pthread_mutex_unlock(&mutex_file_line);
	return true;
}

void*  operator  new(size_t  size) _THROW_BAD_ALLOC
{
	/*if(n_registered_file_line==0){
		return malloc(size);
	}*/

	char source_file[MEMMGR_MAX_FILENAME_LENGTH]={"??"};
	int source_line=0;

	pthread_mutex_lock(&mutex_file_line);//.lock();


	if(n_registered_file_line > 0)
	{
		--n_registered_file_line;
		strcpy(source_file,registered_file[n_registered_file_line]);
		source_line = registered_line[n_registered_file_line];
	}

	//mutex_file_line.unlock();
	pthread_mutex_unlock(&mutex_file_line);


	void *pointer = NULL;


	if((pointer=MEMMGR_malloc(size,source_file,source_line))!=NULL)
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

	if(n_registered_file_line > 0)
	{
		--n_registered_file_line;
		strcpy(source_file,registered_file[n_registered_file_line]);
		source_line = registered_line[n_registered_file_line];
	}

	//mutex_file_line.unlock();
	pthread_mutex_unlock(&mutex_file_line);


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
	pthread_mutex_lock(&mutex_file_line);


	char source_file[MEMMGR_MAX_FILENAME_LENGTH]={"??"};
	int source_line=0;

	if(n_registered_file_line > 0)
	{
		--n_registered_file_line;
		strcpy(source_file,registered_file[n_registered_file_line]);
		source_line = registered_line[n_registered_file_line];
	}

	pthread_mutex_unlock(&mutex_file_line);
	//mutex_file_line.unlock();

	if(pointer == NULL)
	{
		LOG_LEVEL_WARNING(source_file,  source_line,"delete: Try to deallocate NULL pointer!");
		return;
	}

	preheap_allocat  =  GET_PREHEADER(pointer);
	postheap_allocat  =  GET_POSTHEADER(pointer);

	/*if(!MEMMGR_is_pointer_registered((intptr_t)((char *)pointer-sizeof(PointerPreHeapInfo))))
	{
		LOG_LEVEL_ERROR("(%s:%i): allocated_pointer NOT REGISTERED OR POSSIBLE MEMORY CORRUPTION?!?!",source_file,  source_line);
		return;
	}*/


	if(preheap_allocat->pre_crc  !=  postheap_allocat->post_crc)
	{
		LOG_LEVEL_ERROR(source_file,source_line,"delete: Try to deallocate a pointer with CRC error. Pointer corrupted or pointer not managed by this memory manager!");
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


	pthread_mutex_lock(&mutex_file_line);

	char source_file[MEMMGR_MAX_FILENAME_LENGTH]={"??"};
	int source_line(0);

	if(n_registered_file_line > 0)
	{
		--n_registered_file_line;
		strcpy(source_file,registered_file[n_registered_file_line]);
		source_line = registered_line[n_registered_file_line];

	}

	pthread_mutex_unlock(&mutex_file_line);


	if(pointer==NULL)
	{
		LOG_LEVEL_WARNING(source_file,  source_line,"delete[]: Try to deallocate NULL pointer");
		return;
	}

	preheap_allocat  =  GET_PREHEADER(pointer);
	postheap_allocat  =  GET_POSTHEADER(pointer);

	//  Check  headers...
	if(preheap_allocat->pre_crc  !=  postheap_allocat->post_crc)  //  crc  ok  :)
	{
		LOG_LEVEL_ERROR(source_file,  source_line,"delete[]: Try to deallocate a pointer with CRC error. Pointer corrupted or pointer not managed by this memory manager!");
		return;
	}

	/*if(!MEMMGR_is_pointer_registered((intptr_t)((char *)pointer-sizeof(PointerPreHeapInfo))))
	{
		LOG_LEVEL_ERROR("(%s:%i): allocated_pointer NOT REGISTERED WITH MALLOC OR NEW!",source_file,  source_line);
		return;
	}*/

	if(preheap_allocat->type_allocator  !=  NEW_WITH_BRACETS_ALLOCATOR)
	{
		MEMMGR_print_error_on_wrong_deallocate_method(source_file,  source_line,preheap_allocat->type_allocator);
		return;
	}

	MEMMGR_free(pointer,  source_file,  source_line);


}

