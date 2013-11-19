#ifndef _VFS_UITL_H_
#define _VFS_UTIL_H_

#include "common.h"

typedef int (*dir_foreach_item_proc)(const char*fullpath,int dir);

VFS_EXTERN unsigned int		vfs_util_calc_crc32( void*, int );
VFS_EXTERN int				vfs_util_compress_bound( int ,int );
VFS_EXTERN int				vfs_util_compress( int,  const void*, int,void*, int );
VFS_EXTERN int				vfs_util_decompress( int, const void*, int,void*, int );
VFS_EXTERN int				vfs_util_dir_foreach(const char*,dir_foreach_item_proc);

#endif/* _VFS_UTIL_H_ */
