#include "memmgr.h"
#include "common.h"

#undef	new
#undef	delete

#define DEFAULT_CPP_ALIGNMENT	alignof(std::max_align_t)

//#define DEFINE_PUSH_FILE_LINE_TYPE(__type__)
//---------
// DELETE

static char registered_file_new[MEMMGR_MAX_STACK_FILE_LINE][MEMMGR_MAX_FILENAME_LENGTH]={0};
static int 	registered_line_new[MEMMGR_MAX_STACK_FILE_LINE]={-1};
static int 	n_registered_file_line_new=0;
static 		pthread_mutex_t mutex_file_line_new = PTHREAD_MUTEX_INITIALIZER;

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
		MEMMGR_LOG_INFO(__FILE__\
			,__LINE__\
			,"reached max stacked files new");\
	}\
	pthread_mutex_unlock(&mutex_file_line_new);
	return true;\
}

//---------
// DELETE

void*  operator  new(size_t  _size, const char *_file, int _line) _THROW_BAD_ALLOC {

	void *pointer = MEMMGR_malloc_alignment(_size,_file,_line,DEFAULT_CPP_ALIGNMENT);

	if(pointer == NULL){
		throw std::bad_alloc();
	}

	PointerPreHeapInfo  *pre_head = GET_PREHEADER(pointer,DEFAULT_CPP_ALIGNMENT);
	pre_head->type_allocator  =  NEW_ALLOCATOR;


	return  pointer;
}
//--------------------------------------------------------------------------------------------
void*  operator  new[](size_t  _size, const char *_file, int _line) _THROW_BAD_ALLOC {
	void *pointer = NULL;

	if((pointer  =  MEMMGR_malloc_alignment(_size,_file, _line,DEFAULT_CPP_ALIGNMENT))==NULL){
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

void  __cpp_delete__(void  *pointer) _NO_EXCEPT_TRUE
{
	/*PointerPreHeapInfo *preheap_allocat=NULL;
	PointerPostHeapInfo *postheap_allocat=NULL;

	if(pointer == NULL) {
		MEMMGR_LOG_WARNING(__FILE__, __LINE__,"delete: Trying to deallocate NULL pointer!");
		return;
	}

	preheap_allocat  =  GET_PREHEADER(pointer,DEFAULT_CPP_ALIGNMENT);
	postheap_allocat  =  GET_POSTHEADER(pointer,DEFAULT_CPP_ALIGNMENT);

	if(preheap_allocat->pre_crc  !=  postheap_allocat->post_crc)
	{
		MEMMGR_LOG_ERROR(__FILE__, __LINE__,"delete: Trying to deallocate a pointer with CRC error. Either is a corrupted pointer or not managed pointer!");
		return;
	}*/

	MEMMGR_free(pointer,  __FILE__, __LINE__,DEFAULT_CPP_ALIGNMENT,NEW_ALLOCATOR);

}


#if defined(__cpp_sized_deallocation) || (__cplusplus >= 201402L)
void  operator  delete(void  *_pointer, size_t _size) _NO_EXCEPT_TRUE{
	((void)_size);
	//throw std::runtime_error("operator delete(void  *pointer, size_t _size) not implemented");
	__cpp_delete__(_pointer);
}
#endif

void  operator  delete(void  *_pointer) _NO_EXCEPT_TRUE{
	__cpp_delete__(_pointer);
}
//--------------------------------------------------------------------------------------------

void  __cpp_delete_array__(void  *pointer) _NO_EXCEPT_TRUE
{
	PointerPreHeapInfo *preheap_allocat=NULL;
	PointerPostHeapInfo *postheap_allocat=NULL;

	if(pointer==NULL)
	{
		MEMMGR_LOG_WARNING(__FILE__, __LINE__,"delete[]: Trying to deallocate NULL pointer");
		return;
	}

	preheap_allocat  =  GET_PREHEADER(pointer,DEFAULT_CPP_ALIGNMENT);
	postheap_allocat  =  GET_POSTHEADER(pointer,DEFAULT_CPP_ALIGNMENT);

	//  Check  headers...
	if(preheap_allocat->pre_crc  !=  postheap_allocat->post_crc)  //  crc  ok  :)
	{
		MEMMGR_LOG_ERROR(__FILE__, __LINE__,"delete[]: Trying to deallocate a pointer with CRC error. Either is a corrupted pointer or not managed pointer!");
		return;
	}

	MEMMGR_free(pointer,  __FILE__, __LINE__,DEFAULT_CPP_ALIGNMENT,NEW_WITH_BRACETS_ALLOCATOR);

}

#if defined(__cpp_sized_deallocation) || (__cplusplus >= 201402L)
void  operator  delete[](void  *_pointer, size_t _size) _NO_EXCEPT_TRUE{
	((void)_size);
	__cpp_delete_array__(_pointer);

}
#endif

void  operator  delete[](void  *_pointer) _NO_EXCEPT_TRUE{
	__cpp_delete_array__(_pointer);
}

