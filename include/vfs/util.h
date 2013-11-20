#ifndef _VFS_UITL_H_
#define _VFS_UTIL_H_

#include "common.h"

typedef int (*dir_foreach_item_proc)(const char*fullpath,int dir);

VFS_EXTERN unsigned int		vfs_util_calc_crc32( void*, int );

VFS_EXTERN int				vfs_util_compress_bound( int ,int );
VFS_EXTERN int				vfs_util_compress( int,  const void*, int,void*, int );
VFS_EXTERN int				vfs_util_decompress( int, const void*, int,void*, int );

VFS_EXTERN int				vfs_util_dir_foreach(const char*,dir_foreach_item_proc);

VFS_EXTERN char*			vfs_util_path_from(char*);
VFS_EXTERN char*			vfs_util_path_append(char*,char*);
VFS_EXTERN char*			vfs_util_path_combine(char*,const char*,const char*);
VFS_EXTERN char*			vfs_util_path_add_backslash(char*);
VFS_EXTERN char*			vfs_util_path_remove_backslash(char*);
VFS_EXTERN char*			vfs_util_path_remove_filename(char*);
VFS_EXTERN char*			vfs_util_path_remove_extension(char*);

#endif/* _VFS_UTIL_H_ */
