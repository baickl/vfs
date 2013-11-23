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
#include <vfs/file.h>
#include <vfs/vfs.h>
#include "pak.h"
#include "vfs_private.h"
#include <stdio.h>
#include <string.h>

static FILE* sfopen(const char* filename,const char* mode)
{
#ifndef _WIN32
	return fopen(filename,mode);
#else
	FILE* fp = NULL;
	var32 err;

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

var32 vfs_file_exists( const char* file  )
{
    int i,count;
    pak* _pak;

    char*filefullpath;
    char filepath[VFS_MAX_FILENAME+1];

    FILE* fp;

    /************************************************************************/

    if( !file)
        return VFS_FILE_NOT_EXISTS;

    /* 先尝试在包里查找 */
    count = vfs_get_pak_count();
    for( i = 0; i<count; ++i )
    {
        _pak = vfs_get_pak_index(i);
        if( _pak && pak_item_locate(_pak,file) >= 0  )
            return VFS_FILE_EXISTS_IN_PAK;
    }


    /* 判断是否在本地 */
    memset(filepath,0,sizeof(filepath));
    filefullpath = file;
    if( vfs_util_path_combine(filepath,g_vfs->_M_workpath,file) )
    {
        filefullpath = filepath;
    }

    fp = sfopen(filefullpath,"rb");
    if( fp )
    {
        VFS_SAFE_FCLOSE(fp);
        return VFS_FILE_EXISTS_IN_DIR;
    }

    /* 不存在文件 */
    return VFS_FILE_NOT_EXISTS;
}

vfs_file* vfs_file_create(void*buf,uvar64 size)
{
	vfs_file* vff;

	vff = (vfs_file*)malloc(sizeof(vfs_file));
	if( !vff )
	{
		VFS_SAFE_FREE(buf);
		return NULL;
	}

	if( !size)
	{
		vff->_M_buffer = 0;
		vff->_M_size = 0;
		vff->_M_position = 0;
	}
	else
	{
		vff->_M_buffer = (void*)malloc(size);
		if( !vff->_M_buffer )
		{
			VFS_SAFE_FREE(buf);
			return NULL;
		}
		vff->_M_position = 0;
		vff->_M_size = size;

		if( buf )
			memcpy(vff->_M_buffer,buf,(size_t)size);
	}
	
	return vff;
}

vfs_file* vfs_file_open(const char* file )
{
	var32 i,index ;
	var64 size;
	void* buf;
	pak_iteminfo* iteminfo;
	vfs_file* vff;

	FILE* fp;

    char*filefullpath;
    char filepath[VFS_MAX_FILENAME+1];

    int l;

	if( !file )
		return NULL;

	/* 先尝试从包里读取 */
	if( g_vfs )
	{
		for( i = 0; i<g_vfs->_M_count; ++i )
		{
			index = pak_item_locate(g_vfs->_M_paks[i],file);
			if(index < 0 )
				continue;

			iteminfo = pak_item_get_info(g_vfs->_M_paks[i],index);
			if( !iteminfo )
				continue;

			size = iteminfo->_M_size;
			buf = (void*)malloc(size);
			if( !buf )
				return NULL;

			if( VFS_TRUE != pak_item_unpack_index(g_vfs->_M_paks[i],index,buf,size) ) 
			{
				VFS_SAFE_FREE(buf);
				return NULL;
			}

			vff = vfs_file_create(0,0);
			if( !vff )
			{
				VFS_SAFE_FREE(buf);
				return NULL;
			}
			vff->_M_buffer = buf;
			vff->_M_size = size;
			vff->_M_position = 0;

			return vff;
		}
	}


    memset(filepath,0,sizeof(filepath));
    filefullpath = file;
    if( vfs_util_path_combine(filepath,g_vfs->_M_workpath,file) )
    {
        filefullpath = filepath;
    }


	/* 包里读取不出来，在本地读取 */
	fp = sfopen(filefullpath,"rb");
	if( fp )
	{
		VFS_FSEEK(fp,0,SEEK_END);
		size = VFS_FTELL(fp);
		VFS_FSEEK(fp,0,SEEK_SET);
		buf = NULL;

		if( size > 0 )
		{
			buf = (void*)malloc(size);
			if( !buf )
			{
				VFS_SAFE_FCLOSE(fp);
				return NULL;
			}

			if( !VFS_CHECK_FREAD(fp,buf,size) )
			{
				VFS_SAFE_FCLOSE(fp);
				VFS_SAFE_FREE(buf);
				return NULL;
			}

			VFS_SAFE_FCLOSE(fp);
		}

		vff = vfs_file_create(0,0);
		if( !vff )
		{
			VFS_SAFE_FREE(buf);
			return NULL;
		}
		vff->_M_buffer = buf;
		vff->_M_size = size;
		vff->_M_position = 0;

		return vff;
	}

	return NULL;
}

void vfs_file_close(vfs_file* file)
{
	if( file )
	{
		VFS_SAFE_FREE(file->_M_buffer);
		VFS_SAFE_FREE(file);
	}
}


VFS_BOOL vfs_file_eof(vfs_file* file )
{
	if( file && file->_M_position >= file->_M_size )
		return VFS_TRUE;
	else
		return VFS_FALSE;
}

uvar64 vfs_file_tell(vfs_file* file)
{
	if( file )
		return file->_M_position;

	return 0;
}

uvar64 vfs_file_size( vfs_file* file )
{
	if( !file )
		return 0;

	return file->_M_size;
}

const void* vfs_file_data( vfs_file* file )
{
    if( !file )
        return 0;

    return file->_M_buffer;
}

uvar64 vfs_file_seek(vfs_file* file,var64 pos, var32 mod )
{
	uvar64 _pos;
	if( !file )
		return -1;

	if( mod == SEEK_CUR )
	{
		_pos = file->_M_position + pos;
		if( _pos >= 0 && _pos < file->_M_size  )
			file->_M_position = pos;
	}
	else if( mod == SEEK_END )
	{
		_pos = file->_M_size -1 + pos;
		if( _pos >= 0 && _pos < file->_M_size )
			file->_M_position = _pos;
	}
	else
	{
		if( pos >= 0 && pos < (var64)file->_M_size  )
			file->_M_position = pos;
	}

	return vfs_file_tell(file);
}

size_t vfs_file_read( void* buf , size_t size , size_t count , vfs_file*fp )
{

	size_t i;
	size_t realread;
	char* p;

	if(!fp)
		return 0;

	if(!buf)
		return 0;
	
	if(!size || !count )
		return 0;
	
	p = (char*)buf;
	realread = 0;

	for( i = 0; i<count; ++i )
	{
		if( vfs_file_eof(fp) )
			break;

		if( (fp->_M_size - fp->_M_position ) >= size )
		{
			memcpy(p,&((char*)fp->_M_buffer)[fp->_M_position],(size_t)size);
			fp->_M_position += size;
			p += size;
			realread += size;
		}
	}

	return realread;
}

size_t vfs_file_write(void* buf , size_t size , size_t count , vfs_file*fp )
{
	size_t i;
	size_t realwrite;
	char* p;
	
	void* tmp;
	
	if( !fp )
		return 0;

	if( !buf )
		return 0;

	if( !size || !count )
		return 0;

	p = (char*)buf;
	realwrite = 0;
	for( i = 0; i<count; ++i )
	{
		if( (fp->_M_size - fp->_M_position ) < size )
		{
			tmp = (void*)realloc(fp->_M_buffer,fp->_M_size + size );
			if( !tmp )
				break;

			fp->_M_buffer = tmp;
			fp->_M_size += size;
		}

		memcpy(&((char*)fp->_M_buffer)[fp->_M_position],p,(size_t)size);
		fp->_M_position += size;
		p += size;
		realwrite += size;

	}

	return realwrite;
}


VFS_BOOL vfs_file_save(vfs_file* file,const char* saveas)
{

	FILE* fp;
	uvar64 offset;

	uvar64 realsize ;
	char buf[512+1];

	if( !file || !saveas  )
		return VFS_FALSE;
	
	fp = sfopen(saveas,"wb+");
	if( !fp )
		return VFS_FALSE;

	offset = vfs_file_tell(file);
	while( !vfs_file_eof(file) )
	{
		realsize = vfs_file_read(buf,1,512,file);
		if( realsize > 0 )
		{
			buf[realsize] = 0;
			if( fwrite(buf,1,(size_t)realsize,fp) != realsize )
			{
				VFS_SAFE_FCLOSE(fp);
				vfs_file_seek(file,offset,SEEK_SET);
				remove(saveas);
				return VFS_FALSE;
			}
		}

		VFS_SAFE_FCLOSE(fp);
		vfs_file_seek(file,offset,SEEK_SET);
	}

	VFS_SAFE_FCLOSE(fp);
	return VFS_TRUE;
}
