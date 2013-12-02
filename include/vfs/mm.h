#ifndef _VFS_MM_H_
#define _VFS_MM_H_

#include "base.h"

struct vfs_mm
{    
    void* (*malloc)(size_t);
    void* (*realloc)(void*,size_t);
    void  (*free)(void*);
};

VFS_EXTERN uvar64   vfs_mm_mem_total_used();
VFS_EXTERN uvar64   vfs_mm_mem_pak_used();
VFS_EXTERN uvar64   vfs_mm_mem_hashtable_used();

#endif
