#ifndef    __JGT_RAM_H__
#define    __JGT_RAM_H__

#define __HAVE_OGL__


#ifdef __MEMMANAGER__

#ifndef  __FUNCTION__
	#define	__FUNCTION__  "??"
#endif  

enum{
        UNKNOWN_ALLOCATE=0,
        MALLOC_ALLOCATOR,  //  by  default
        NEW_ALLOCATOR,  
        NEW_WITH_BRACETS_ALLOCATOR,  
        SDL_SURFACE_ALLOCATOR,
        SDL_SOCKET_ALLOCATOR,
        SDL_SOUND_ALLOCATOR,
        MAX_ALLOCATE_TYPES
};



typedef  void  (*  void_function_void_pointer)(const  char  *module,const  char  *function,  int  line,  
                                                                    void  *pointer);


#define ZG_MEM_DISABLE_REGISTER_MEMORYLEAKS	 	MEM_setEnableRegisterMemoryLeaks(0)
#define ZG_MEM_ENABLE_REGISTER_MEMORYLEAKS 		MEM_setEnableRegisterMemoryLeaks(1)


//------------------------------------------------------------------------------------------------------------

void        MEM_setEnableRegisterMemoryLeaks(int enable);
void        MEM_ViewStatus();
void        MEM_PrintAllocatedPointers(const char *_file, int line);
void        MEM_Init();
void        MEM_FreeAllAllocatedPointers( );
void        MEM_ListAllocatedPointers();
#define  MEM_New(type,n_items)              ((type  *)JGT_malloc(sizeof(type)*n_items,  __FILE__,  __LINE__))
//------------------------------------------------------------------------------------------------------------
void        JGT_AddReservedPointer(void  *pointer,  void_function_void_pointer deallocator, const  char  *filename,  const  char  *function,  int  line);
int          JGT_DeleteReservedPointer(void  *pointer, const  char  *filename,  const  char  *function,  int  line);
int 	    JGT_PreAllocatedPointerExist(void *pointer, const  char  *filename,  const  char  *function,  int  line);
void        *JGT_malloc(size_t  size,  const  char  *filename,  int  line);
void        *JGT_realloc(void *ptr, size_t  size,  const  char  *filename,  int  line);
void        JGT_addPointer(void  *pointer,  int  type_allocated_pointer,  const  char  *filename,  int  line);
void        JGT_PreCheckFree(void  *pointer,  const  char  *filename,  int  row  );

#ifdef free
#undef free
#endif

#ifdef malloc
#undef malloc
#endif

#ifdef realloc
#undef realloc
#endif


#define  free(p)                               JGT_PreCheckFree(p,__FILE__,  __LINE__)
#define  malloc(size)	                        JGT_malloc(size,  __FILE__,  __LINE__)
#define  realloc(ptr,size)	                        JGT_realloc(ptr,size,  __FILE__,  __LINE__)
//------------------------------------------------------------------------------------------------------------

#ifdef  __cplusplus

bool	m_setOwner(const  char  *file,  const  unsigned  int  line,  const  char  *func);

#ifdef  __GNUC__
	void*  operator  new(size_t  size);
	void*  operator  new(size_t  size,    const  char  *file,  const  unsigned  int  line,  const  char  *func);
	
	void*  operator  new[](size_t  size);
	void*  operator  new[](size_t  size,const  char  *file,  const  unsigned  int  line,  const  char  *func);
	
	void    operator  delete(void  *p) throw();
	void    operator  delete(void  *p,const  char  *file,  const  unsigned  int  line,  const  char  *func);
	
	void    operator  delete[](void  *p) throw();
	void    operator  delete[](void  *p,const  char  *file,  const  unsigned  int  line,  const  char  *func);

#endif  

#endif  

//---------------------------------  REDEFINE  MALLOC  FUNCTIONS  HERE  ------------------

#include                  "MEM_ram_Nommgr.h"



#define  malloc(p)                                      	JGT_malloc(p,__FILE__,  __LINE__)//MEM_Malloc
#define  realloc(p,s)                                      	JGT_realloc(p,s,__FILE__,  __LINE__)//MEM_Malloc
#define  free(p)                                         	JGT_PreCheckFree(p,__FILE__,  __LINE__)//MEM_Free
#ifdef  __cplusplus

#define	new		                                        (m_setOwner(__FILE__,__LINE__,__FUNCTION__),false)  ?  NULL  :  new
#define	delete		                                	(m_setOwner(__FILE__,__LINE__,__FUNCTION__) , false)? throw : delete

#endif
#endif
#endif

#ifndef __MEMMANAGER__

#define ZG_MEM_DISABLE_REGISTER_MEMORYLEAKS
#define ZG_MEM_ENABLE_REGISTER_MEMORYLEAKS

#endif  
