#ifndef _VFS_COMMON_H_
#define _VFS_COMMON_H_

#include "typedef.h"


/* 
 * 函数导出定义
 * */
#define VFS_EXTERN extern

/*
 * 常用宏定义
 * */
#define VFS_SAFE_FCLOSE(x)		if(x){fclose(x);x=NULL;}
#define VFS_SAFE_FREE(x)		if(x){free((void*)x);x=NULL;}

#define VFS_CHECK_FREAD(f,b,l)	(fp ? (fread((void*)b,1,l,f)        != l ? 0 : 1) : 0 )
#define VFS_CHECK_FWRITE(f,b,l) (fp ? (fwrite((const void*)b,1,l,f) != l ? 0 : 1) : 0 )

/* 
 * MACRO DEFINE
 * */

#define VFS_VERSION			(1)

#define VFS_MAX_FILENAME	(512)

#define VFS_COMPRESS_NONE	(0x0000)
#define VFS_COMPRESS_BZIP2	(0x0001)

#define VFS_TRUE			(1)
#define VFS_FALSE			(0)


#endif
