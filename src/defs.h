#ifndef __MEMMGR_DEFS_H__
#define __MEMMGR_DEFS_H__

#ifdef  __cplusplus

#ifdef __APPLE__
	#define _NO_EXCEPT_TRUE _NOEXCEPT
#else
	#define _THROW_BAD_ALLOC
	#define _NO_EXCEPT_TRUE noexcept(true)
#endif


#endif

#endif
