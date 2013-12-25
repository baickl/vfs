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
#include "bz2compress_plugin.h"
#include <string.h>

#define BZ_NO_STDIO
#include "../bzip2/bzlib.h"

static const VFS_CHAR* bz2compress_plugin_get_plugin_name()
{
    return "VFS_PLUGIN_COMPRESS_BZIP2";
}

static const VFS_CHAR* bz2compress_plugin_get_plugin_version()
{
    return "1.0.0.1";
}

static const VFS_CHAR* bz2compress_plugin_get_plugin_description()
{
    return "VFS_PLUGIN_COMPRESS_BZIP2";
}

static const VFS_CHAR* bz2compress_plugin_get_plugin_copyright()
{
    return "Copyright(C) baickl(baickl@gmail.com)";
}

static const VFS_CHAR* bz2compress_plugin_get_plugin_support()
{
    return "Email:baickl@gmail.com";
}


static VFS_SIZE bz2compress_plugin_compress_bound( VFS_SIZE srclen )
{
    return (VFS_SIZE)(srclen*1.01+600);
}

static VFS_SIZE bz2compress_plugin_compress(const VFS_VOID*src,VFS_SIZE srcsize,VFS_VOID*dst,VFS_SIZE dstsize)
{
    VFS_SIZE r;
    VFS_SIZE compressed_size = dstsize;

    r = BZ2_bzBuffToBuffCompress((VFS_CHAR*)dst,(unsigned int*)&compressed_size,(VFS_CHAR*)src,(unsigned int)srcsize,9,3,30);
    if( r != BZ_OK)
        return 0;
    return compressed_size;
}

static VFS_SIZE bz2compress_plugin_decompress(const VFS_VOID*src,VFS_SIZE srcsize,VFS_VOID*dst,VFS_SIZE dstsize)
{
    VFS_SIZE r;
    VFS_SIZE uncompressed_size = dstsize;

    r = BZ2_bzBuffToBuffDecompress((VFS_CHAR*)dst,(unsigned int*)&uncompressed_size,(VFS_CHAR*)src,(unsigned int)srcsize,0,2);
    if( r != BZ_OK )
        return 0;
    return uncompressed_size;
}

/************************************************************************/
/* 导出函数                                                    */
/************************************************************************/
vfs_plugin vfs_get_plugin_compress_bz2compress()
{
    vfs_plugin plugin;

    memset(&plugin,0,sizeof(plugin));

    plugin.type = PLUGIN_COMPRESS;

    plugin.info.get_plugin_name             = bz2compress_plugin_get_plugin_name;
    plugin.info.get_plugin_version          = bz2compress_plugin_get_plugin_version;
    plugin.info.get_plugin_description      = bz2compress_plugin_get_plugin_description;
    plugin.info.get_plugin_copyright        = bz2compress_plugin_get_plugin_copyright;
    plugin.info.get_plugin_support          = bz2compress_plugin_get_plugin_support;

    plugin.plugin.compress.compress_bound   = bz2compress_plugin_compress_bound;
    plugin.plugin.compress.compress         = bz2compress_plugin_compress;
    plugin.plugin.compress.decompress       = bz2compress_plugin_decompress;

    return plugin;
}