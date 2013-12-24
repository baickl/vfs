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
typedef var32 (*archive_item_foreach_proc)(vfs_archive archive ,const char* filename, uvar64 size);

enum
{
    PLUGIN_ARCHIVE,
    PLUGIN_COMPRESS,
    PLUGIN_ENCRYPT
};

typedef struct vfs_plugin_info
{
    const char*             (*get_plugin_name)();
    const char*             (*get_plugin_version)();
    const char*             (*get_plugin_description)();
    const char*             (*get_plugin_copyright)();
    const char*             (*get_plugin_support)();
}vfs_plugin_info;

typedef struct vfs_plugin_archive
{
    VFS_BOOL                (*archive_check_suppert)(const char* archive);
    vfs_archive             (*archive_open)(const char* archive,const char* prefix,const char* passwd);
    void                    (*archive_close)( vfs_archive archive);
    const char*             (*archive_get_name)( vfs_archive archive);
    var32                   (*archive_get_item_count)( vfs_archive archive);
    VFS_BOOL                (*archive_foreach_item)( vfs_archive archive,archive_item_foreach_proc proc);
    VFS_BOOL                (*archive_locate_item)(vfs_archive archive,const char* filename,uvar64* osize);
    VFS_BOOL                (*archive_unpack_item_by_filename)( vfs_archive archive, const char* filename, void* buf, uvar64 bufsize);

}vfs_plugin_archive;

typedef struct vfs_plugin_compress
{
    var32                   (*compress_bound)( var8 type,var32 len );
    uvar64                  (*compress)( var32 type , const void *src, uvar64 srclen ,void *dst, uvar64 dstlen);
    uvar64                  (*decompress)( var32 type , const void *src, uvar64 srclen ,void *dst, uvar64 dstlen);
}vfs_plugin_compress;

typedef struct vfs_plugin_encrypt
{
    size_t                  (*encrypt)(const void*src,size_t srclen,void*dst,size_t dstlen );
    size_t                  (*decrypt)(const void*src,size_t srclen,void*dst,size_t dstlen );
}vfs_plugin_encrypt;

typedef union vfs_plugin_obj
{
    vfs_plugin_archive      archive;
    vfs_plugin_compress     compress;
    vfs_plugin_encrypt      encrypt;
}vfs_plugin_obj;

typedef struct vfs_plugin
{
    var32                   type;
    vfs_plugin_info         info;
    vfs_plugin_obj          plugin;
}vfs_plugin;

#endif/*_VFS_PLUGIN_H_*/
