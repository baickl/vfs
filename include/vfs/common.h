#ifndef _VFS_COMMON_H_
#define _VFS_COMMON_H_

#include "typedef.h"

#define VFS_EXTERN extern

#define VFS_SAFE_FCLOSE(x)		if(x){fclose(x);x=NULL;}
#define VFS_SAFE_FREE(x)		if(x){free((void*)x);x=NULL;}

#ifndef _WIN32 
	#define VFS_FSEEK	fseeko
	#define VFS_FTELL	ftello
	#define stricmp		strcasecmp
#else
	#define VFS_FSEEK	_fseeki64
	#define VFS_FTELL	_ftelli64
#endif 


#define VFS_CHECK_FREAD(f,b,l)	(f ? (fread((void*)b,1,(size_t)l,f)        != l ? 0 : 1) : 0 )
#define VFS_CHECK_FWRITE(f,b,l) (f ? (fwrite((const void*)b,1,(size_t)l,f) != l ? 0 : 1) : 0 )

#define VFS_VERSION				(1)

#define VFS_MAX_FILENAME		(512)

#define VFS_COMPRESS_NONE		(0x0000)
#define VFS_COMPRESS_BZIP2		(0x0001)

#define VFS_BOOL				int
#define VFS_TRUE				(1)
#define VFS_FALSE				(0)

#ifndef _WIN32

#endif 




#endif/*_VFS_COMMON_H_*/
