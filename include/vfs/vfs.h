#ifndef _VFS_VFS_H_
#define _VFS_VFS_H_

#include "pak.h"

/* VFS  */
typedef struct vfs_t
{
	int			_M_count;
	int			_M_maxcount;
	pak**		_M_paks;
}vfs;

/* VFS_FILE */
typedef struct vfs_file_t
{
	int			_M_size;
	int			_M_position;
	void*		_M_buffer;
}vfs_file;

/* VFS系统 */
VFS_EXTERN int				vfs_create();
VFS_EXTERN void				vfs_destroy();

VFS_EXTERN int				vfs_pak_add(const char*);
VFS_EXTERN int				vfs_pak_del(const char*);

VFS_EXTERN int				vfs_pak_get_count();
VFS_EXTERN pak*				vfs_pak_get_index(int);
VFS_EXTERN pak*				vfs_pak_get_name(const char*);


/* VFS对FILE相关函数模拟 */
VFS_EXTERN vfs_file*		vfs_fopen(const char*,const char*);
VFS_EXTERN void				vfs_fclose(vfs_file*);

VFS_EXTERN int				vfs_feof(vfs_file*);
VFS_EXTERN int				vfs_ftell(vfs_file*);
VFS_EXTERN int				vfs_fseek(vfs_file*,int,int);
VFS_EXTERN size_t			vfs_fread(void*,size_t,size_t,vfs_file*);
VFS_EXTERN size_t			vfs_fwrite(void*,size_t,size_t,vfs_file*);

#endif
