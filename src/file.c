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
#include "vfs_private.h"
#include <stdio.h>
#include <string.h>

static FILE* sfopen(const char* filename,const char* mode)
{
#ifdef __linux__
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
    vfs_archive_obj* _archive;
    uvar64 size;

    const char*filefullpath;
    char filepath[VFS_MAX_FILENAME+1];

    FILE* fp;

    /************************************************************************/

    if( !file)
        return VFS_FILE_NOT_EXISTS;

    /* 先尝试在包里查找 */
    count = vfs_get_archive_count();
    for( i = 0; i<count; ++i )
    {
        _archive = vfs_get_archive_index(i);
        if( _archive && _archive->plugin->plugin_archive_item_locate(_archive->archive,file,&size) !=NULL  )
            return VFS_FILE_EXISTS_IN_ARCHIVE;
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

vfs_file* vfs_file_create(void *buf,uvar64 size)
{
	vfs_file* vff;

    if( !g_vfs )
        return NULL;

	vff = (vfs_file*)malloc(sizeof(vfs_file));
	if( !vff )
	{
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
            free(vff);
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
	var32 i ;
	var64 size;
	void* buf;
    vfs_archive_obj* p;
	vfs_file* vff;

	FILE* fp;

    const char*filefullpath;
    char filepath[VFS_MAX_FILENAME+1];

    if( !g_vfs )
        return NULL;

	if( !file )
		return NULL;

	/* 先尝试从包里读取 */
    for( i = 0; i<g_vfs->_M_count; ++i )
    {
        p = g_vfs->_M_archives[i];
        if( VFS_TRUE != p->plugin->plugin_archive_item_locate(p->archive,file,&size) )
            continue;

        buf = (void*)malloc(size);
        if( !buf )
            return NULL;

        if( VFS_TRUE != p->plugin->plugin_archive_item_unpack_filename(p->archive,file,buf,size) ) 
        {
            free(buf);
            return NULL;
        }

        vff = vfs_file_create(0,0);
        if( !vff )
        {
            if(buf)
            {
                free(buf);
                buf = NULL;
            }
            return NULL;
        }
        vff->_M_buffer = buf;
        vff->_M_size = size;
        vff->_M_position = 0;

        return vff;
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
			buf = (void*)malloc(size);
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
                    free(buf);
                    buf = NULL;
                }
				return NULL;
			}

			VFS_SAFE_FCLOSE(fp);
		}

		vff = vfs_file_create(0,0);
		if( !vff )
		{
			if(buf){
                free(buf);
                buf = NULL;
            }
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
		if(file->_M_buffer)
        {
            free(file->_M_buffer);
            file->_M_buffer = NULL;
        }
		free(file);
        file = NULL;
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
	size_t realsize;
    size_t realcount;
	char* p;
    char* cursor;

	if(!fp || !buf || !size || !count)
		return 0;
    
    realcount = (size_t)((fp->_M_size - fp->_M_position)/size);
    if( realcount <= 0  )
        return 0;

    realcount = realcount<count?realcount:count;

    p = (char*)buf;
    cursor = &((char*)fp->_M_buffer)[fp->_M_position];
    realsize = realcount* size;
    fp->_M_position += realsize;
    memcpy(p,cursor,(size_t)realsize);
    return realcount;
}

size_t vfs_file_write(void* buf , size_t size , size_t count , vfs_file*fp )
{
	size_t realwrite;
    size_t realcount;
    size_t needsize;
	char* p,*tmp;
	
	if( !fp || !buf || !size || !count )
		return 0;

    realcount = (size_t)((fp->_M_size - fp->_M_position)/size);
    if(realcount < count )
    {
        needsize = (count - realcount)*size;
        if( fp->_M_size == 0 )
        {
            tmp = (void*)malloc(needsize );
            if( tmp )
            {
                fp->_M_buffer = tmp;
                fp->_M_size = needsize;

                realcount = count;
            }
        }
        else
        {
            tmp = (void*)realloc(fp->_M_buffer,fp->_M_size + needsize );
            if( tmp )
            {
                fp->_M_buffer = tmp;
                fp->_M_size += needsize;

                realcount = count;
            }
        }
        
    }

    if( realcount == 0 )
        return 0;

    p = (char*)buf;
    realwrite = realcount*size;

	memcpy(&((char*)fp->_M_buffer)[fp->_M_position],p,(size_t)realwrite);
	fp->_M_position += realwrite;
	return realcount;
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
    vfs_file_seek(file,0,SEEK_SET);
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
	}

    VFS_SAFE_FCLOSE(fp);
    vfs_file_seek(file,offset,SEEK_SET);
	return VFS_TRUE;
}
