#ifndef __MEMMGR_H__
#define __MEMMGR_H__

#define MEMMGR_VERSION_MAJOR	1
#define MEMMGR_VERSION_MINOR	4
#define MEMMGR_VERSION_PATCH	0

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

#if defined(_WIN32) && !defined(__CYGWIN__)
# if defined(BUILD_STATIC_LIBS)
#  define MEMMGR_DLL_EXPORT
# elif defined(BUILD_SHARED_LIBS)
#  define MEMMGR_DLL_EXPORT __declspec(dllexport)
# else
#  define MEMMGR_DLL_EXPORT __declspec(dllimport)
# endif
#elif defined(__OS2__) && defined(__WATCOMC__) && defined(__SW_BD)
#  define MEMMGR_DLL_EXPORT __declspec(dllexport)
#elif (defined(__GNUC__) || defined(__clang__) || defined(__HP_cc)) && defined(MEMMGR_SYM_VISIBILITY)
# define MEMMGR_DLL_EXPORT __attribute__((visibility ("default")))
#elif defined(__SUNPRO_C) && defined(MEMMGR_LDSCOPE_GLOBAL)
# define MEMMGR_DLL_EXPORT __global
#elif defined(EMSCRIPTEN)
# include <emscripten.h>
# define MEMMGR_DLL_EXPORT EMSCRIPTEN_KEEPALIVE
# define MEMMGR_DLL_EXPORT_VAR
#else
# define MEMMGR_DLL_EXPORT
#endif

#ifndef  __FUNCTION__
	#define	__FUNCTION__  "??"
#endif

MEMMGR_DLL_EXPORT	void		MEMMGR_enableLog(bool _enable);
MEMMGR_DLL_EXPORT	void        *MEMMGR_malloc(size_t  _size,  const  char  *_filename,  int  _line);
MEMMGR_DLL_EXPORT	void        *MEMMGR_realloc(void *_ptr, size_t  _size,  const  char  *_filename,  int  _line);
MEMMGR_DLL_EXPORT	void 		*MEMMGR_calloc(size_t  _n_items,size_t  _size_item,  const  char  *_filename,  int  _line);
MEMMGR_DLL_EXPORT	void        MEMMGR_free_from_malloc(void  *_ptr,  const  char  *_filename,  int  _line);
MEMMGR_DLL_EXPORT	void		MEMMGR_free_c_pointer(void  *_ptr);
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
		//#include            <iostream>
		#include 			<functional>
		#include 			<sstream>
		#include 			<memory>
		#include 			<regex>   // include "regex" first to avoid warnings error: invalid pure specifier (only ‘= 0’ ...
		#include			<cstddef>
		// check std features in order to include or not
		#include			<set>
		#include			<array>
		#include			<fstream>
		#include			<iomanip>
		#include			<chrono>
		#include			<cstring>
		#include 			<codecvt>
		#include 			<locale>
		#if defined __has_include
		#  if __has_include (<optional.h>)
		#    include <optional.h>
		#  endif
		#endif

MEMMGR_DLL_EXPORT	bool		MEMMGR_push_file_line_new(const char *_filename,  int  _line);
MEMMGR_DLL_EXPORT			bool		MEMMGR_push_file_line_delete(const char *_filename,  int  _line);
MEMMGR_DLL_EXPORT			bool		MEMMGR_push_file_line_new_array(const char *_filename,  int  _line);
MEMMGR_DLL_EXPORT			bool		MEMMGR_push_file_line_delete_array(const char *_filename,  int  _line);


MEMMGR_DLL_EXPORT			void*  		operator  new(size_t  _size) _THROW_BAD_ALLOC;
MEMMGR_DLL_EXPORT			void*  		operator  new[](size_t  _size) _THROW_BAD_ALLOC;
MEMMGR_DLL_EXPORT			void   		operator  delete(void  *_ptr)  _NO_EXCEPT_TRUE;
#if (__cplusplus >= 201402L) // delete (void  *_ptr, std::size_t _size) was introduced on std::c++14
MEMMGR_DLL_EXPORT			void   		operator  delete(void  *_ptr, std::size_t _size)  _NO_EXCEPT_TRUE;
MEMMGR_DLL_EXPORT			void   		operator  delete[](void  *_ptr, std::size_t _size)  _NO_EXCEPT_TRUE;
#endif

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
