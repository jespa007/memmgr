#ifndef __MEMMGR_H__
#define __MEMMGR_H__

#define MEMMGR_VERSION_MAJOR	1
#define MEMMGR_VERSION_MINOR	5
#define MEMMGR_VERSION_PATCH	0

#include	<stdbool.h>
#include	"defs.h"

#include "memmgr_api.h"

#ifndef  __FUNCTION__
	#define	__FUNCTION__  "??"
#endif


#define  malloc(p)                                      	MEMMGR_malloc(p,__FILE__,  __LINE__)
#define  calloc(n,s)                                      	MEMMGR_calloc(n,s,__FILE__,  __LINE__)
#define  realloc(p,s)                                      	MEMMGR_realloc(p,s,__FILE__,  __LINE__)
#define  free(p)                                         	MEMMGR_free_from_malloc(p,__FILE__,  __LINE__)


//------------------------------------------------------------------------------------------------------------


#ifdef  __cplusplus

#ifdef __APPLE__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wkeyword-macro"
#endif
	#define new new(__FILE__, __LINE__)

#ifdef __APPLE__
#pragma GCC diagnostic pop
#endif

#endif

#endif
