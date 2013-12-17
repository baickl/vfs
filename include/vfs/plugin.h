/***********************************************************************************
 * Copyright (c) 2013, baickl(baickl@gmail.com)
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 * 
 * * Neither the name of the {organization} nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***********************************************************************************/
#ifndef _VFS_PLUGIN_H_
#define _VFS_PLUGIN_H_

#include "base.h"

typedef void* vfs_archive;
typedef var32 (*plugin_item_foreach_proc)(vfs_archive archive ,const char* filename, uvar64 size);

typedef struct vfs_plugin_s
{
    const char*     (*plugin_archive_get_plugin_name)();
    VFS_BOOL        (*plugin_archive_check_type)(const char* archive);

    vfs_archive     (*plugin_archive_open)(const char* archive,const char* prefix);
    void            (*plugin_archive_close)( vfs_archive archive);

    const char*     (*plugin_archive_get_name)( vfs_archive archive);

    var32           (*plugin_archive_item_get_count)( vfs_archive archive);
    VFS_BOOL        (*plugin_archive_item_foreach)( vfs_archive archive,plugin_item_foreach_proc proc);
    VFS_BOOL        (*plugin_archive_item_locate)(vfs_archive archive,const char* filename,uvar64* osize);

    VFS_BOOL        (*plugin_archive_item_unpack_filename)( vfs_archive archive, const char* filename, void* buf, uvar64 bufsize);

}vfs_plugin;


#endif/*_VFS_PLUGIN_H_*/
