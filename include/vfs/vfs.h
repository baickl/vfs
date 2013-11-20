#ifndef _VFS_VFS_H_
#define _VFS_VFS_H_

#include "pak.h"

/* VFS  */
typedef struct vfs_t
{
	var32		_M_count;
	var32		_M_maxcount;
	pak**		_M_paks;
}vfs;

/* VFS_FILE */
typedef struct vfs_file_t
{
	uvar64		_M_size;
	uvar64		_M_position;
	void*		_M_buffer;
}vfs_file;

/* VFS系统 */
VFS_EXTERN VFS_BOOL			vfs_create();
VFS_EXTERN void				vfs_destroy();

VFS_EXTERN VFS_BOOL			vfs_pak_add(const char*);
VFS_EXTERN VFS_BOOL			vfs_pak_del(const char*);
VFS_EXTERN var32			vfs_pak_get_count();
VFS_EXTERN pak*				vfs_pak_get_index(var32);
VFS_EXTERN pak*				vfs_pak_get_name(const char*);


/* VFS对FILE相关函数模拟 */
VFS_EXTERN vfs_file*		vfs_fcreate(void*buf,uvar64 size);
VFS_EXTERN vfs_file*		vfs_fopen(const char*,const char*);
VFS_EXTERN void				vfs_fclose(vfs_file*);
VFS_EXTERN VFS_BOOL			vfs_feof(vfs_file*);
VFS_EXTERN uvar64			vfs_ftell(vfs_file*);
VFS_EXTERN uvar64			vfs_fseek(vfs_file*,var64,var32);
VFS_EXTERN uvar64			vfs_fsize(vfs_file*);
VFS_EXTERN uvar64			vfs_fread(void*,uvar64,uvar64,vfs_file*);
VFS_EXTERN uvar64			vfs_fwrite(void*,uvar64,uvar64,vfs_file*);
VFS_EXTERN VFS_BOOL			vfs_fflush(vfs_file*,const char*);

#endif
