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
#include <vfs/util.h>

#include "crc32/crc32.h"
#define BZ_NO_STDIO
#include "bzip2/bzlib.h"
#include <stdio.h>
#include <memory.h>
#include <sys/types.h>

#ifndef _WIN32
#include <dirent.h>
#else
#include <io.h>  
#include <direct.h>
#endif 



void bz_internal_error ( int errcode )
{
}

uvar32 vfs_util_calc_crc32(void*buf,var32 size)
{
	return calc_crc32(buf,size);
}

var32 vfs_util_compress_bound( var8 compresstype, var32 srclen )
{
	switch(compresstype)
	{
	case VFS_COMPRESS_BZIP2:
		return (var32)(srclen*1.01+600);

	case VFS_COMPRESS_NONE:
	default:
		return srclen;
	}
}

uvar64 vfs_util_compress(var32 compresstype, const void*src,uvar64 srcsize,void*dst,uvar64 dstsize)
{
	uvar64 r;
	uvar64 compressed_size = dstsize;

	switch(compresstype)
	{
	case VFS_COMPRESS_BZIP2:
		r = BZ2_bzBuffToBuffCompress((char*)dst,(unsigned int*)&compressed_size,(char*)src,(unsigned int)srcsize,9,3,30);
		if( r != BZ_OK)
			return 0;
		return compressed_size;

	case VFS_COMPRESS_NONE:
	default:
		return 0;
	}
}

uvar64 vfs_util_decompress(var32 compresstype,const void*src,uvar64 srcsize,void*dst,uvar64 dstsize)
{
	uvar64 r;
	uvar64 uncompressed_size = dstsize;
	switch(compresstype)
	{
	case VFS_COMPRESS_BZIP2:
		r = BZ2_bzBuffToBuffDecompress((char*)dst,(unsigned int*)&uncompressed_size,(char*)src,(unsigned int)srcsize,0,2);
		if( r != BZ_OK )
			return 0;
		return uncompressed_size;
	case VFS_COMPRESS_NONE:
	default:
		return 0;
	}
}


#ifndef _WIN32
VFS_BOOL vfs_util_dir_foreach(const char* path,dir_foreach_item_proc proc)
{
	DIR* dir;
	struct dirent *entry = NULL;

	char find_full[VFS_MAX_FILENAME+1];
	char path_temp[VFS_MAX_FILENAME+1];

	var32 rt;

	if( !vfs_util_path_clone(find_full,path))
		return VFS_FALSE;
	
	dir = opendir(path);
	if( NULL == dir )
		return VFS_FALSE;

	if( NULL == proc )
		return VFS_FALSE;

	while( (entry=readdir(dir)) != NULL )
	{
		if( entry->d_type & 4 )
		{
			if( strcmp(entry->d_name,".")  == 0 || strcmp(entry->d_name,"..") == 0 )
				continue;

			memset(path_temp,0,sizeof(path_temp));
			if( !vfs_util_path_combine(path_temp,path,entry->d_name) )
				goto LB_ERROR;

			rt = proc(path_temp,1);
			switch(rt)
			{
			case DIR_FOREACH_IGNORE:
				break;
			case DIR_FOREACH_BREAK:
				goto FIND_BREAK;
			case DIR_FOREACH_CONTINUE:
			default:
				{
					if( 0 == vfs_util_dir_foreach(path_temp,proc) )
						goto LB_ERROR;
				}
				break;
			}		
		}
		else
		{
			memset(path_temp,0,sizeof(path_temp));
			if( !vfs_util_path_combine(path_temp,path,entry->d_name) )
				goto LB_ERROR;

			rt = proc(path_temp,0);
			switch(rt)
			{
			case DIR_FOREACH_BREAK:
				goto FIND_BREAK;
			}		
		}
	}

FIND_BREAK:
	closedir(dir);
	return VFS_TRUE;

LB_ERROR:
	closedir(dir);
	return VFS_FALSE;

}

#else

VFS_BOOL vfs_util_dir_foreach(const char* path,dir_foreach_item_proc proc)
{
	char find_full[VFS_MAX_FILENAME+1] = {0};
	char path_temp[VFS_MAX_FILENAME+1] = {0};

	var32 rt;

	long hFile=0;  
	struct _finddata_t fileinfo; 

	if( !vfs_util_path_combine(find_full,path,"*") )
		return VFS_FALSE;

	if((hFile=_findfirst(find_full,&fileinfo)) != -1)
	{
		do
		{
			if((fileinfo.attrib &_A_SUBDIR)) 
			{
				if(strcmp(fileinfo.name,".")!= 0 && strcmp(fileinfo.name,"..") != 0 ) 
				{
					memset(path_temp,0,sizeof(path_temp));
					if( !vfs_util_path_combine(path_temp,path,fileinfo.name) )
						goto LB_ERROR;

					rt = proc(path_temp,1);
					switch(rt)
					{
					case DIR_FOREACH_IGNORE:
						break;
					case DIR_FOREACH_BREAK:
						goto FIND_BREAK;
					case DIR_FOREACH_CONTINUE:
					default:
						{
							if( 0 == vfs_util_dir_foreach(path_temp,proc) )
								goto LB_ERROR;
						}
						break;
					}		
				}
			}
			else
			{
				memset(path_temp,0,sizeof(path_temp));
				if( !vfs_util_path_combine(path_temp,path,fileinfo.name) )
					goto LB_ERROR;

				rt = proc(path_temp,0);
				switch(rt)
				{
				case DIR_FOREACH_BREAK:
					goto FIND_BREAK;
				}		

			}  
		}while(_findnext(hFile,&fileinfo) == 0);  

FIND_BREAK:
		_findclose(hFile);  
	}

	return VFS_TRUE;

LB_ERROR:
	if( hFile )
		_findclose(hFile);  
	return VFS_FALSE;

}

#endif


char* vfs_util_path_checkfix(char* path )
{
	char*p;
	if( !path )
		return path;

	p = path;
	while( p && *p != 0 )
	{
		if( *p == '\\' )
			*p = '/';

		++p;
	}

	return path;
}


char* vfs_util_path_clone(char*out_path,const char*path )
{
	if( !out_path || !path )
		return NULL;
	if( strlen(path) <= 0 )
		return NULL;

	strcpy(out_path,path);
	return vfs_util_path_checkfix(out_path);
}

char* vfs_util_path_append(char* path ,char* append )
{
	char *p ;
	if( !path || !append)
		return NULL;

	p = append;
	while( p && *p != 0 )
	{
		if(*p != '\\' || *p != '/' )
			break;

		++p;
	}

	if( !p || *p == 0 )
		return NULL;

	vfs_util_path_add_backslash( path );
	
	strcat(path,p);
	return path;
}

char* vfs_util_path_join(char* path ,char* join )
{
	if( !path || !join)
		return NULL;

	strcat(path,join);
	return vfs_util_path_checkfix(path);
}

char* vfs_util_path_combine(char* path ,const char* dir ,const char* append )
{
	if( !path || !dir || !append )
		return NULL;

	if( !vfs_util_path_clone(path,dir) )
		return NULL;

	return vfs_util_path_append( path,append );
}

char* vfs_util_path_add_backslash(char* path )
{
	var32 len;
	if( !path )
		return NULL;

	vfs_util_path_remove_backslash(path);

	len = strlen(path);
	if( len <= 0 )
		return path;

	strcat(path,"/");
	return path;

}

char* vfs_util_path_remove_backslash(char* path )
{
	var32 len;
	if( !path )
		return path;

	len = strlen(path);
	if( len <= 0 )
		return path;

	while( len > 0 && (path[len-1] == '/' || path[len-1] == '\\' )  )
	{
		path[len-1] = 0;
		--len;
	}

	return path;

}

char* vfs_util_path_remove_filename(char* path )
{
	return path;
}

char* vfs_util_path_remove_extension(char* path )
{
	return path;
}
