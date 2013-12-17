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
#ifndef _VFS_VFS_PRIVATE_H_
#define _VFS_VFS_PRIVATE_H_

#include <vfs/plugin.h>


typedef struct vfs_archive_obj_s
{
    vfs_archive             archive;
    vfs_plugin*             plugin;
}vfs_archive_obj;


typedef struct vfs_t
{
    var32                   _M_count;
    var32                   _M_maxcount;
    vfs_archive_obj**       _M_archives;

    var32                   _M_plugins_count;
    var32                   _M_plugins_maxcount;
    vfs_plugin**            _M_plugins;

    char                    _M_workpath[VFS_MAX_FILENAME];
}vfs;

VFS_EXTERN vfs             *g_vfs;

VFS_EXTERN var32            vfs_get_archive_count( );
VFS_EXTERN vfs_archive_obj* vfs_get_archive_index( var32 );
VFS_EXTERN vfs_archive_obj* vfs_get_archive_name( const char* );

#endif/*_VFS_VFS_PRIVATE_H_*/
