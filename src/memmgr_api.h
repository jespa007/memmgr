#ifndef __MEMMGR_API_H__
#define __MEMMGR_API_H__

#if defined(_WIN32) && !defined(__CYGWIN__)
# if defined(MEMMGR_STATIC_LIBS)
#  define MEMMGR_DLL_EXPORT
# elif defined(MEMMGR_SHARED_LIBS)
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


#ifdef __cplusplus
extern "C" {
#endif
	MEMMGR_DLL_EXPORT	void		MEMMGR_enableLog(bool _enable);
	MEMMGR_DLL_EXPORT	void        *MEMMGR_malloc(size_t  _size,  const  char  *_filename,  int  _line);
	MEMMGR_DLL_EXPORT	void        *MEMMGR_realloc(void *_ptr, size_t  _size,  const  char  *_filename,  int  _line);
	MEMMGR_DLL_EXPORT	void 		*MEMMGR_calloc(size_t  _n_items,size_t  _size_item,  const  char  *_filename,  int  _line);
	MEMMGR_DLL_EXPORT	void        MEMMGR_free_from_malloc(void  *_ptr,  const  char  *_filename,  int  _line);
	MEMMGR_DLL_EXPORT	void		MEMMGR_free_c_pointer(void  *_ptr);
#ifdef __cplusplus
}
#endif

#ifdef  __cplusplus

	#include            <new>
	#include			<cstddef>


	MEMMGR_DLL_EXPORT 			void* 		operator  new(std::size_t _size) _THROW_BAD_ALLOC;
	MEMMGR_DLL_EXPORT			void*  		operator  new(size_t  _size,const char *_file,int _line) _THROW_BAD_ALLOC;
	MEMMGR_DLL_EXPORT 			void* 		operator  new[](std::size_t _size) _THROW_BAD_ALLOC;
	MEMMGR_DLL_EXPORT			void*  		operator  new[](size_t  _size,const char *_file,int _line) _THROW_BAD_ALLOC;



	MEMMGR_DLL_EXPORT			void   		operator  delete(void  *_ptr)  _NO_EXCEPT_TRUE;
	MEMMGR_DLL_EXPORT 			void 		operator delete[](void *_ptr) _NO_EXCEPT_TRUE;

	#if defined(__cpp_sized_deallocation) || (__cplusplus >= 201402L)
		MEMMGR_DLL_EXPORT			void   		operator  delete(void  *_ptr, std::size_t _size)  _NO_EXCEPT_TRUE;
		MEMMGR_DLL_EXPORT			void   		operator  delete[](void  *_ptr, std::size_t _size)  _NO_EXCEPT_TRUE;
	#endif

#endif

#endif
