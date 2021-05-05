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
		LOG_LEVEL_INFO("reached max stacked files!");
		//return false;
	}

	pthread_mutex_unlock(&mutex_file_line);
	return true;
}

void*  operator  new(size_t  size)
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


	void * ret_ptr=NULL;
	void *pointer = MEMMGR_malloc(size,source_file,source_line);


	if(pointer!=NULL)
	{
		PointerPreHeapInfo  *pre_head  =  GET_PREHEADER(pointer);
		pre_head->type_allocator  =  NEW_ALLOCATOR;
	}

	ret_ptr=pointer;

	return  ret_ptr;
}
//--------------------------------------------------------------------------------------------
void*  operator  new[](size_t  size)
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

	void * ret_ptr=NULL;
	void *pointer = NULL;

	pointer  =  MEMMGR_malloc(size,source_file, source_line);
	PointerPreHeapInfo  *pre_head  =  GET_PREHEADER(pointer);
	pre_head->type_allocator  =  NEW_WITH_BRACETS_ALLOCATOR;
	ret_ptr=pointer;

	return  ret_ptr;

}
//--------------------------------------------------------------------------------------------
void  operator  delete(void  *pointer) noexcept(true)
{
	/*if(n_registered_file_line==0){
		free(pointer);
		return;
	}*/


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

	if(pointer != NULL)
	{
		if(!MEMMGR_is_pointer_registered((intptr_t)((char *)pointer-sizeof(PointerPreHeapInfo))))
		{
			LOG_LEVEL_ERROR("(%s:%i): allocated_pointer NOT REGISTERED OR POSSIBLE MEMORY CORRUPTION?!?!",source_file,  source_line);
		}
		else
		{

			PointerPreHeapInfo   *pre_head   =  GET_PREHEADER(pointer);
			PointerPostHeapInfo  *post_head  =  GET_POSTHEADER(pointer);

			if(pre_head->pre_crc  ==  post_head->post_crc)
			{
				if(pre_head->type_allocator  ==  NEW_ALLOCATOR)
				{
					MEMMGR_free(pointer,  source_file,  source_line);
				}
				else  //  error
				{
					MEMMGR_print_error_on_wrong_deallocate_method(pre_head->type_allocator,  source_file,  source_line);
				}
			}
			else
			{
				LOG_LEVEL_ERROR("(%s:%i): CRC  error!",source_file,  source_line);
			}
		}
	}
	else
	{
		LOG_LEVEL_ERROR("ERROR:  NULL  pointer  to  deallocate  at  filename  \"%s\"  line  %i.",source_file,  source_line);
	}




}
//--------------------------------------------------------------------------------------------
void  operator  delete[](void  *pointer) noexcept(true)
{
	/*if(n_registered_file_line==0){
		free(pointer);
		return;
	}*/


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

	//mutex_file_line.unlock();

	if(pointer!=NULL)
	{

		if(!MEMMGR_is_pointer_registered((intptr_t)((char *)pointer-sizeof(PointerPreHeapInfo))))
		{
			LOG_LEVEL_ERROR("(%s:%i): allocated_pointer NOT REGISTERED WITH MALLOC OR NEW!",source_file,  source_line);
		}
		else
		{

			PointerPreHeapInfo  *pre_head  =  GET_PREHEADER(pointer);

			if(pre_head->type_allocator  ==  NEW_WITH_BRACETS_ALLOCATOR)
			{
				MEMMGR_free(pointer,  source_file,  source_line);
			}
			else  //  error
			{
				MEMMGR_print_error_on_wrong_deallocate_method(pre_head->type_allocator,  source_file,  source_line);
			}
		}
	}
	else
	{
		LOG_LEVEL_ERROR("ERROR:  NULL  pointer  to  deallocate  at  filename  \"%s\"  line  %i",source_file,  source_line);
	}

	/*if(n_registered_file_line > 0){
		n_registered_file_line--;
	}*/

}


