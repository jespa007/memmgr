#ifndef __MEMMORY_MANNAGER_H__
#define __MEMMORY_MANNAGER_H__

#define MEMMGR_MAJOR_VERSION 	1
#define MEMMGR_MINOR_VERSION 	2
#define MEMMGR_PATCH_VERSION 	0

#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<memory.h>
#include	<stdarg.h>
#include	<stdint.h>
#include	<pthread.h>
#include	<stdbool.h>


#ifdef _WIN32
#include	<windows.h>
#endif

#ifndef  __FUNCTION__
	#define	__FUNCTION__  "??"
#endif

	void		MEMMGR_enableLog(bool _enable);
	void        *MEMMGR_malloc(size_t  _size,  const  char  *_filename,  int  _line);
	void        *MEMMGR_realloc(void *_ptr, size_t  _size,  const  char  *_filename,  int  _line);
	void 		*MEMMGR_calloc(size_t  _n_items,size_t  _size_item,  const  char  *_filename,  int  _line);
	void        MEMMGR_free_from_malloc(void  *_ptr,  const  char  *_filename,  int  _line);
	void		MEMMGR_free_c_pointer(void  *_ptr);
	#define  malloc(p)                                      	MEMMGR_malloc(p,__FILE__,  __LINE__)
	#define  calloc(n,s)                                      	MEMMGR_calloc(n,s,__FILE__,  __LINE__)
	#define  realloc(p,s)                                      	MEMMGR_realloc(p,s,__FILE__,  __LINE__)
	#define  free(p)                                         	MEMMGR_free_from_malloc(p,__FILE__,  __LINE__)


	//------------------------------------------------------------------------------------------------------------

	#ifdef  __cplusplus

#ifdef __APPLE__
	#define _NO_EXCEPT_TRUE _NOEXCEPT
#else
	#define _THROW_BAD_ALLOC
	#define _NO_EXCEPT_TRUE noexcept(true)
#endif

		#include          	<vector>
		#include 			<map>
		#include 			<unordered_map> // include "unordered_map" first to avoid warnings error: invalid pure specifier (only ‘= 0’ ...
		#include            <new>
		#include            <iostream>
		#include 			<functional>
		#include 			<sstream>
		#include 			<memory>
		#include 			<regex>   // include "regex" first to avoid warnings error: invalid pure specifier (only ‘= 0’ ...

		bool		MEMMGR_push_file_line_new(const char *_filename,  int  _line);
		bool		MEMMGR_push_file_line_delete(const char *_filename,  int  _line);
		bool		MEMMGR_push_file_line_new_array(const char *_filename,  int  _line);
		bool		MEMMGR_push_file_line_delete_array(const char *_filename,  int  _line);


		void*  		operator  new(size_t  _size) _THROW_BAD_ALLOC;
		void*  		operator  new[](size_t  _size) _THROW_BAD_ALLOC;
		void   		operator  delete(void  *_ptr)  _NO_EXCEPT_TRUE;
		void   		operator  delete[](void  *_ptr)  _NO_EXCEPT_TRUE;

#ifdef __APPLE__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wkeyword-macro"
#endif
		#define	new		                                    (MEMMGR_push_file_line_new(__FILE__,__LINE__),false)?NULL:new

		///
		/// Delete it fails in this situations:
		///
		/// 1. When deletes a NULL pointer: It invokes MEMMGR_push_file_line but delete() or delete[]() operator is not called, so
		///    it increments its counter delete refs
		/// 2. The procedure on delete is the following:
		///    1st: It captures current file line by calling MEMMGR_push_file_line_delete
		///    2nd: It calls object destructor (i.e Object::~Object)
		///    3rd: It call overrided delete[],delete operator
		///
		///    On the 1st and 3rd steps there's a mutex_lock that blocks temporally counter and file/line refs to set and get current
		///    file/line where delete operator was invoked. During the destructor call (2nd step)  destructor can call other deletes
		//     and become mismatched file/line

		#define	delete		  		                        (MEMMGR_push_file_line_delete(__FILE__,__LINE__),false)?abort(): delete

#ifdef __APPLE__
#pragma GCC diagnostic pop
#endif

	#endif

#endif
