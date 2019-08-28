#pragma once


#ifdef __MEMMGR__

#ifndef  __FUNCTION__
	#define	__FUNCTION__  "??"
#endif


	#define ZG_MEM_DISABLE_REGISTER_MEMORYLEAKS	 	MEM_setEnableRegisterMemoryLeaks(0)
	#define ZG_MEM_ENABLE_REGISTER_MEMORYLEAKS 		MEM_setEnableRegisterMemoryLeaks(1)

	void 		MEMMGR_print_status();
	void        *MEMMGR_malloc(size_t  size,  const  char  *filename,  int  line);
	void        *MEMMGR_realloc(void *ptr, size_t  size,  const  char  *filename,  int  line);
	void        MEMMGR_free_from_malloc(void  *pointer,  const  char  *filename,  int  line);

	#define  malloc(p)                                      	MEMMGR_malloc(p,__FILE__,  __LINE__)
	#define  realloc(p,s)                                      	MEMMGR_realloc(p,s,__FILE__,  __LINE__)
	#define  free(p)                                         	MEMMGR_free_from_malloc(p,__FILE__,  __LINE__)

	//------------------------------------------------------------------------------------------------------------

	#ifdef  __cplusplus

		bool		MEMMGR_push_file_line(const char *absolute_filename,  int  line);

		void*  		operator  new(size_t  size);
		void*  		operator  new[](size_t  size);
		void   		operator  delete(void  *p) throw();
		void   		operator  delete[](void  *p) throw();

		#define	new		                                        (MEMMGR_push_file_line(__FILE__,__LINE__), false)?  NULL  :  new
		#define	delete		                                	(MEMMGR_push_file_line(__FILE__,__LINE__), false)? throw  : delete

	#endif


	void test_dicotomic();


#else // MEMMGR not defined...

	#define ZG_MEM_DISABLE_REGISTER_MEMORYLEAKS
	#define ZG_MEM_ENABLE_REGISTER_MEMORYLEAKS

#endif
