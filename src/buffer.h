#ifndef _VFS_BUFFER_H_
#define _VFS_BUFFER_H_

struct vfs_buffer
{
    size_t  _M_size;
    void*   _M_buf;
};

VFS_EXTERN void     vfs_buffer_init(struct vfs_buffer*);
VFS_EXTERN void*    vfs_buffer_alloc(struct vfs_buffer*,size_t);
VFS_EXTERN void     vfs_buffer_cleanup(struct vfs_buffer*);

#endif
