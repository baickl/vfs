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
#ifndef _VFS_PLUGIN_ARCHIVE_H_
#define _VFS_PLUGIN_ARCHIVE_H_

#include "../base.h"

/*****************************************************************************
 * 前置声明
 */
typedef VFS_VOID*             vfs_archive;
typedef VFS_INT32             (*archive_item_foreach_proc)(vfs_archive archive ,const VFS_CHAR* filename, VFS_UINT64 size);

/*****************************************************************************
 * 文件包插件定义
 */
typedef struct vfs_plugin_archive
{
    VFS_BOOL                  (*archive_check_suppert)(const VFS_CHAR* archive);
    vfs_archive               (*archive_open)(const VFS_CHAR* archive,const VFS_CHAR* prefix,const VFS_CHAR* passwd);
    VFS_VOID                  (*archive_close)( vfs_archive archive);
    const VFS_CHAR*           (*archive_get_name)( vfs_archive archive);
    VFS_INT32                 (*archive_get_item_count)( vfs_archive archive);
    VFS_BOOL                  (*archive_foreach_item)( vfs_archive archive,archive_item_foreach_proc proc);
    VFS_BOOL                  (*archive_locate_item)(vfs_archive archive,const VFS_CHAR* filename,VFS_UINT64* osize);
    VFS_BOOL                  (*archive_unpack_item_by_filename)( vfs_archive archive, const VFS_CHAR* filename, VFS_VOID* buf, VFS_UINT64 bufsize);
}vfs_plugin_archive;

#endif/*_VFS_PLUGIN_ARCHIVE_H_*/
