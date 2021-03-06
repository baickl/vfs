﻿/***********************************************************************************
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
#include <vfs/stream.h>
#include <vfs/vfs.h>
#include <vfs/util.h>
#include "vfs_private.h"
#include "pool.h"
#include <stdio.h>
#include <string.h>

/************************************************************************/
/* VFS文件结构                                                  */
/************************************************************************/
struct vfs_stream
{
    VFS_UINT64		            _M_size;
    VFS_UINT64		            _M_position;
    VFS_VOID*		            _M_buffer;
};


static FILE* sfopen(const VFS_CHAR* filename,const VFS_CHAR* mode)
{
#ifdef __linux__
	return fopen(filename,mode);
#else
	FILE* fp = NULL;
	VFS_INT32 err;

	err = fopen_s(&fp,filename,mode);
	if( err == 0 )
	{
		return fp;
	}
	else
	{
		return NULL;
	}
#endif
}



static struct vfs_stream* vfs_stream_new()
{
    struct vfs_stream *stream;

    stream = (struct vfs_stream *)vfs_pool_malloc(sizeof(struct vfs_stream));
    if( !stream )
        return NULL;

    stream->_M_buffer = 0;
    stream->_M_position = 0;
    stream->_M_size = 0;

    return stream;
}

static VFS_VOID vfs_stream_delete( struct vfs_stream* stream )
{
    if( stream )
    {
        vfs_pool_free(stream);
    }
}

struct vfs_stream* vfs_stream_create( VFS_VOID *buf,VFS_UINT64 size)
{
    struct vfs_stream *stream;

    stream = vfs_stream_new();
    if( !stream )
        return NULL;

	if( size)
	{
		stream->_M_buffer = (VFS_VOID*)vfs_pool_malloc((VFS_SIZE)size);
		if( !stream->_M_buffer )
		{
            vfs_stream_close(stream);
			return NULL;
		}
		stream->_M_position = 0;
		stream->_M_size = size;

		if( buf )
			memcpy(stream->_M_buffer,buf,(VFS_SIZE)size);
	}
	
	return stream;
}

struct vfs_stream* vfs_stream_open( const VFS_CHAR* file )
{
	VFS_INT32 i ;
	VFS_INT64 size;
	VFS_VOID* buf;
    vfs_archive_obj* p;

	FILE* fp;

    const VFS_CHAR*filefullpath;
    VFS_CHAR filepath[VFS_MAX_FILENAME+1];
    struct vfs_stream* stream;

	/* 先尝试从包里读取 */
    for( i = 0; i<g_vfs->_M_count; ++i )
    {
        p = g_vfs->_M_archives[i];
        if( VFS_TRUE != p->plugin->plugin.archive.archive_locate_item(p->archive,file,&size) )
            continue;

        buf = (VFS_VOID*)vfs_pool_malloc((VFS_SIZE)size);
        if( !buf )
        {
            return NULL;
        }

        if( VFS_TRUE != p->plugin->plugin.archive.archive_unpack_item_by_filename(p->archive,file,buf,size) ) 
        {
            return NULL;
        }

        stream = vfs_stream_create(buf,size);
        vfs_pool_free(buf);
        return stream;
    }

	/* 包里读取不出来，在本地读取 */
    memset(filepath,0,sizeof(filepath));
    filefullpath = file;
    if( vfs_util_path_combine(filepath,g_vfs->_M_workpath,file) )
    {
        filefullpath = filepath;
    }

	fp = sfopen(filefullpath,"rb");
	if( fp )
	{
		VFS_FSEEK(fp,0,SEEK_END);
		size = VFS_FTELL(fp);
		VFS_FSEEK(fp,0,SEEK_SET);
		buf = NULL;

		if( size > 0 )
		{
			buf = (VFS_VOID*)vfs_pool_malloc((VFS_SIZE)size);
			if( !buf )
			{
				VFS_SAFE_FCLOSE(fp);
				return NULL;
			}

			if( !VFS_CHECK_FREAD(fp,buf,size) )
			{
				VFS_SAFE_FCLOSE(fp);
				if(buf)
                {
                    vfs_pool_free(buf);
                    buf = NULL;
                }
				return NULL;
			}

			VFS_SAFE_FCLOSE(fp);
		}

        stream = vfs_stream_create(buf,size);
        vfs_pool_free(buf);
        return stream;
	}

	return NULL;
}

VFS_VOID vfs_stream_close(struct vfs_stream* stream)
{
	if( !stream )
        return;

	if(stream->_M_buffer)
    {
        vfs_pool_free(stream->_M_buffer);
        stream->_M_buffer = NULL;
    }

    stream->_M_position = 0;
    stream->_M_size = 0;

    vfs_stream_delete(stream);
}


VFS_BOOL vfs_stream_eof(struct vfs_stream* stream )
{
    if( !stream )
        return VFS_FALSE;

	if( stream->_M_position >= stream->_M_size )
		return VFS_TRUE;
	else
		return VFS_FALSE;
}

VFS_UINT64 vfs_stream_tell(struct vfs_stream* stream)
{
	if( stream )
		return stream->_M_position;

	return 0;
}

VFS_UINT64 vfs_stream_size( struct vfs_stream* stream )
{
	if( !stream )
		return 0;

	return stream->_M_size;
}

const VFS_VOID* vfs_stream_data( struct vfs_stream* stream )
{
    if( !stream )
        return 0;

    return stream->_M_buffer;
}

VFS_UINT64 vfs_stream_seek(struct vfs_stream* stream,VFS_INT64 pos, VFS_INT32 mod )
{
	VFS_UINT64 _pos;
	if( !stream )
		return -1;

	if( mod == SEEK_CUR )
	{
		_pos = stream->_M_position + pos;
		if( _pos >= 0 && _pos < stream->_M_size  )
			stream->_M_position = pos;
	}
	else if( mod == SEEK_END )
	{
		_pos = stream->_M_size -1 + pos;
		if( _pos >= 0 && _pos < stream->_M_size )
			stream->_M_position = _pos;
	}
	else
	{
		if( pos >= 0 && pos < (VFS_INT64)stream->_M_size  )
			stream->_M_position = pos;
	}

	return vfs_stream_tell(stream);
}

VFS_SIZE vfs_stream_read(struct vfs_stream*stream, VFS_VOID* buf , VFS_SIZE size , VFS_SIZE count)
{
	VFS_SIZE realsize;
    VFS_SIZE realcount;
	VFS_CHAR* p;
    VFS_CHAR* cursor;

	if(!stream || !buf || !size || !count)
		return 0;
    
    realcount = (VFS_SIZE)((vfs_stream_size(stream) - vfs_stream_tell(stream))/size);
    if( realcount <= 0  )
        return 0;

    realcount = realcount<count?realcount:count;

    p = (VFS_CHAR*)buf;
    cursor = &((VFS_CHAR*)vfs_stream_data(stream))[vfs_stream_tell(stream)];
    realsize = realcount* size;
    stream->_M_position += realsize;
    memcpy(p,cursor,(VFS_SIZE)realsize);
    return realcount;
}

VFS_SIZE vfs_stream_write( struct vfs_stream*stream, VFS_VOID* buf , VFS_SIZE size , VFS_SIZE count )
{
	VFS_SIZE realwrite;
    VFS_SIZE realcount;
    VFS_SIZE needsize;
	VFS_CHAR* p,*tmp;
	
	if( !stream || !buf || !size || !count )
		return 0;

    realcount = (VFS_SIZE)((vfs_stream_size(stream) - vfs_stream_tell(stream))/size);
    if(realcount < count )
    {
        needsize = (count - realcount)*size;
        if( stream->_M_size == 0 )
        {
            tmp = (VFS_VOID*)vfs_pool_malloc(needsize );
            if( tmp )
            {
                stream->_M_buffer = tmp;
                stream->_M_size = needsize;

                realcount = count;
            }
        }
        else
        {
            tmp = (VFS_VOID*)vfs_pool_realloc(stream->_M_buffer,(VFS_SIZE)(stream->_M_size + needsize) );
            if( tmp )
            {
                stream->_M_buffer = tmp;
                stream->_M_size += needsize;

                realcount = count;
            }
        }
    }

    if( realcount == 0 )
        return 0;

    p = (VFS_CHAR*)buf;
    realwrite = realcount*size;

	memcpy(&((VFS_CHAR*)stream->_M_buffer)[stream->_M_position],p,(VFS_SIZE)realwrite);
	stream->_M_position += realwrite;
	return realcount;
}


VFS_BOOL vfs_stream_save(struct vfs_stream* stream,const VFS_CHAR* saveas)
{

	FILE* fp;
	VFS_UINT64 offset;

	VFS_UINT64 realsize ;
	VFS_CHAR buf[512+1];

	if( !stream || !saveas  )
		return VFS_FALSE;
	
	fp = sfopen(saveas,"wb+");
	if( !fp )
		return VFS_FALSE;

	offset = vfs_stream_tell(stream);
    vfs_stream_seek(stream,0,SEEK_SET);
	while( !vfs_stream_eof(stream) )
	{
		realsize = vfs_stream_read(stream,buf,1,512);
		if( realsize > 0 )
		{
			buf[realsize] = 0;
			if( fwrite(buf,1,(VFS_SIZE)realsize,fp) != realsize )
			{
				VFS_SAFE_FCLOSE(fp);
				vfs_stream_seek(stream,offset,SEEK_SET);
				remove(saveas);
				return VFS_FALSE;
			}
		}
	}

    VFS_SAFE_FCLOSE(fp);
    vfs_stream_seek(stream,offset,SEEK_SET);
	return VFS_TRUE;
}
