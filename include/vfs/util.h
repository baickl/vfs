#ifndef _VFS_UITL_H_
#define _VFS_UTIL_H_

#include "common.h"

#define DIR_FOREACH_IGNORE		0	/* 返回FOREACH，本次结果不处理，如果是遇到目录返回，则表示不进入目录继续FOREACH */
#define DIR_FOREACH_CONTINUE	1   /* 继续FOREACH，主要用于处理目录进入*/
#define DIR_FOREACH_BREAK		2   /* 中断FOREACH，一般是找到想要的文件夹或是文件后，不需要继续FOREACH了 */

typedef int (*dir_foreach_item_proc)(const char*fullpath,int dir);

VFS_EXTERN unsigned int		vfs_util_calc_crc32( void*, int );

VFS_EXTERN int				vfs_util_compress_bound( int ,int );
VFS_EXTERN int				vfs_util_compress( int,  const void*, int,void*, int );
VFS_EXTERN int				vfs_util_decompress( int, const void*, int,void*, int );

VFS_EXTERN int				vfs_util_dir_foreach(const char*,dir_foreach_item_proc);

VFS_EXTERN char*			vfs_util_path_checkfix(char*);
VFS_EXTERN char*			vfs_util_path_clone(char*,const char*);
VFS_EXTERN char*			vfs_util_path_append(char*,char*);
VFS_EXTERN char*			vfs_util_path_join(char*,char*);
VFS_EXTERN char*			vfs_util_path_combine(char*,const char*,const char*);
VFS_EXTERN char*			vfs_util_path_add_backslash(char*);
VFS_EXTERN char*			vfs_util_path_remove_backslash(char*);
VFS_EXTERN char*			vfs_util_path_remove_filename(char*);
VFS_EXTERN char*			vfs_util_path_remove_extension(char*);

#endif/* _VFS_UTIL_H_ */
