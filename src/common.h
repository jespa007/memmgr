#ifndef __MEMMGR_COMMON_H__
#define __MEMMGR_COMMON_H__

#include	<stdbool.h>
#include	<stdint.h>


#define	MAX_MEMPOINTERS					80000
#define	MEMMGR_MAX_FILENAME_LENGTH		256
#define MEMMGR_MAX_STACK_FILE_LINE		32

#define SIZEOF_ALIGNED_HEADER(_block_alignment)         (((sizeof(PointerPreHeapInfo) + (_block_alignment) - 1) / (_block_alignment)) * (_block_alignment))

#define GET_PREHEADER(p,a)								((PointerPreHeapInfo    *)((uint8_t  *)p-SIZEOF_ALIGNED_HEADER(a)))
#define GET_POINTER(header_ptr,a)						((void    *)(((uint8_t  *)header_ptr+SIZEOF_ALIGNED_HEADER(a))))
#define	GET_SIZE_PTR(p,a)								(GET_PREHEADER(p,a)->size)
#define GET_POSTHEADER(p,a)								((PointerPostHeapInfo  *)((uint8_t  *)(p)+(GET_SIZE_PTR(p,a))))
#define	KEY_NOT_FOUND									-1

typedef enum{
	UNKNOWN_ALLOCATOR=0,
	MALLOC_ALLOCATOR,  //  by  default
	NEW_ALLOCATOR,
	NEW_WITH_BRACETS_ALLOCATOR,
	MAX_ALLOCATE_TYPES
}ALLOCATOR_TYPE;

typedef enum{
	LOG_TYPE_INFO=0
	,LOG_TYPE_WARNING
	,LOG_TYPE_ERROR
}LogType;


#define MEMMGR_LOG_INFOF(file,line,s, ...)		MEMMGR_log(LOG_TYPE_INFO,file,line,s, __VA_ARGS__)
#define MEMMGR_LOG_INFO(file,line,s)			MEMMGR_LOG_INFOF(file,line,s,NULL)

#define MEMMGR_LOG_WARNINGF(file,line,s, ...)	MEMMGR_log(LOG_TYPE_WARNING,file,line,s, __VA_ARGS__)
#define MEMMGR_LOG_WARNING(file,line,s)   		MEMMGR_LOG_WARNINGF(file,line,s,NULL)

#define MEMMGR_LOG_ERRORF(file,line,s, ...)		MEMMGR_log(LOG_TYPE_ERROR,file,line,s, __VA_ARGS__)
#define MEMMGR_LOG_ERROR(file,line,s)   		MEMMGR_LOG_ERRORF(file,line,s,NULL)

//--------------------------------------------------------------------------------------------
// STRUCTS

typedef  struct{
	uintptr_t 	*ptr;
	char  		filename[MEMMGR_MAX_FILENAME_LENGTH+1];
	int  		line;
}InfoAllocatedPointer;

typedef  struct{
	int		type_allocator;
	int		offset_mempointer_table;
	char	filename[MEMMGR_MAX_FILENAME_LENGTH+1];  //  base    		-16-256
	int		line;          					//  base          	-16
	size_t	size;                      		//  base          	-8
	int		pre_crc;                		//  base          	-4
	size_t  alignment;
}PointerPreHeapInfo;

typedef  struct{
	int		post_crc;
}PointerPostHeapInfo;

#ifdef __cplusplus
extern "C" {
#endif
	void 		MEMMGR_enableLog(bool _enable_log);
	void  		MEMMGR_get_filename(char  *filename, const char *absolute_filename);
	void  		MEMMGR_log(LogType _log_type, const char *_file, int _line, const  char  *string_text, ...);
	void 	*	MEMMGR_malloc_alignment(size_t  _size, size_t _aligment,  const  char  *_absolute_filename,  int  _line);
	void  		MEMMGR_free(void  *pointer,  const  char  *filename,  int  line, size_t _alignment, int _expected_allocator);
#ifdef __cplusplus
}
#endif

#endif
