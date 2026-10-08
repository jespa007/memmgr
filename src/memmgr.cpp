#include <cstddef>
#include <new>

#include "common.h"
#include "defs.h"


#define DEFAULT_CPP_ALIGNMENT	alignof(std::max_align_t)

//---------
// NEW

void* operator new(size_t _size) _THROW_BAD_ALLOC {
    void *pointer = MEMMGR_malloc_alignment(
        _size,
        DEFAULT_CPP_ALIGNMENT,
        "??",
        0
    );

    if(pointer == NULL){
        throw std::bad_alloc();
    }

    PointerPreHeapInfo *pre_head = GET_PREHEADER(
        pointer,
        DEFAULT_CPP_ALIGNMENT
    );

    pre_head->type_allocator = NEW_ALLOCATOR;

    return pointer;
}

void* operator new[](size_t _size) _THROW_BAD_ALLOC {
    void *pointer = MEMMGR_malloc_alignment(
        _size,
        DEFAULT_CPP_ALIGNMENT,
        "??",
        0
    );

    if(pointer == NULL){
        throw std::bad_alloc();
    }

    PointerPreHeapInfo *pre_head = GET_PREHEADER(
        pointer,
        DEFAULT_CPP_ALIGNMENT
    );

    pre_head->type_allocator = NEW_WITH_BRACETS_ALLOCATOR;

    return pointer;
}

void*  operator  new(size_t  _size, const char *_file, int _line) _THROW_BAD_ALLOC {

	void *pointer = MEMMGR_malloc_alignment(_size,DEFAULT_CPP_ALIGNMENT,_file,_line);

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

	if((pointer  =  MEMMGR_malloc_alignment(_size,DEFAULT_CPP_ALIGNMENT,_file, _line))==NULL){
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

void __cpp_delete_array__(void *pointer) _NO_EXCEPT_TRUE
{
    MEMMGR_free(
        pointer,
        __FILE__,
        __LINE__,
        DEFAULT_CPP_ALIGNMENT,
        NEW_WITH_BRACETS_ALLOCATOR
    );
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

