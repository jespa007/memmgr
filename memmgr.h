#ifndef __MEMMORY_MANNAGER_H__
#define __MEMMORY_MANNAGER_H__

#define MEMMGR_MAJOR_VERSION 	1
#define MEMMGR_MINOR_VERSION 	1
#define MEMMGR_PACTH_VERSION 	4

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

	void 		MEMMGR_print_status(void);
	void        *MEMMGR_malloc(size_t  size,  const  char  *filename,  int  line);
	void        *MEMMGR_realloc(void *ptr, size_t  size,  const  char  *filename,  int  line);
	void        MEMMGR_free_from_malloc(void  *pointer,  const  char  *filename,  int  line);
	void		MEMMGR_free_c_pointer(void  *pointer);
	#define  malloc(p)                                      	MEMMGR_malloc(p,__FILE__,  __LINE__)
	#define  realloc(p,s)                                      	MEMMGR_realloc(p,s,__FILE__,  __LINE__)
	#define  free(p)                                         	MEMMGR_free_from_malloc(p,__FILE__,  __LINE__)

	//------------------------------------------------------------------------------------------------------------

	#ifdef  __cplusplus
		#include          	<vector>
		#include 			<map>
		#include            <new>
		#include            <iostream>
		#include 			<functional>
		#include 			<string>
		#include 			<sstream>
		#include 			<regex>   // keep regex to avoid warnings error: invalid pure specifier (only ‘= 0’ ...



		bool		MEMMGR_push_file_line(const char *absolute_filename,  int  line);

		void*  		operator  new(size_t  size);
		void*  		operator  new[](size_t  size);
		void   		operator  delete(void  *p)  noexcept(true);
		void   		operator  delete[](void  *p)  noexcept(true);



		#define	new		                                    (MEMMGR_push_file_line(__FILE__,__LINE__),false)?NULL:new
		#define	delete		  		                        (MEMMGR_push_file_line(__FILE__,__LINE__),false)?abort(): delete



	#endif

#endif
