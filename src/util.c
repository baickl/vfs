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
#include <vfs/util.h>

#include "crc32/crc32.h"
#define BZ_NO_STDIO
#include "bzip2/bzlib.h"
#include <stdio.h>
#include <memory.h>
#include <sys/types.h>

#ifdef __linux__
#include <dirent.h>
#else
#include <io.h>  
#include <direct.h>
#endif 



VFS_VOID bz_internal_error ( int errcode )
{
}

VFS_UINT32 vfs_util_calc_crc32(VFS_VOID*buf,VFS_INT32 size)
{
	return calc_crc32(buf,size);
}

#ifdef __linux__
VFS_BOOL vfs_util_dir_foreach(const VFS_CHAR* path,dir_foreach_item_proc proc,VFS_VOID*p)
{
	DIR* dir;
	struct dirent *entry = NULL;

	VFS_CHAR find_full[VFS_MAX_FILENAME+1];
	VFS_CHAR path_temp[VFS_MAX_FILENAME+1];

	VFS_INT32 rt;

    if( !path || !proc )
        return VFS_FALSE;

	if( !vfs_util_path_clone(find_full,path))
		return VFS_FALSE;
	
	dir = opendir(path);
	if( NULL == dir )
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

			rt = proc(path_temp,1,p);
			switch(rt)
			{
			case VFS_FOREACH_IGNORE:
				break;
			case VFS_FOREACH_BREAK:
				goto FIND_BREAK;
            case VFS_FOREACH_PROC_ERROR:
                goto LB_ERROR;
			case VFS_FOREACH_CONTINUE:
			default:
				{
					if( 0 == vfs_util_dir_foreach(path_temp,proc,p) )
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

			rt = proc(path_temp,0,p);
			switch(rt)
			{
			case VFS_FOREACH_BREAK:
				goto FIND_BREAK;
            case VFS_FOREACH_PROC_ERROR:
                goto LB_ERROR;
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

VFS_BOOL vfs_util_dir_foreach(const VFS_CHAR* path,dir_foreach_item_proc proc,VFS_VOID*p)
{
	VFS_CHAR find_full[VFS_MAX_FILENAME+1] = {0};
	VFS_CHAR path_temp[VFS_MAX_FILENAME+1] = {0};

	VFS_INT32 rt;

	long hFile=0;  
	struct _finddata_t fileinfo; 

    if( !path || !proc )
        return VFS_FALSE;

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

					rt = proc(path_temp,1,p);
					switch(rt)
					{
					case VFS_FOREACH_IGNORE:
						break;
					case VFS_FOREACH_BREAK:
						goto FIND_BREAK;
                    case VFS_FOREACH_PROC_ERROR:
                        goto LB_ERROR;
					case VFS_FOREACH_CONTINUE:
					default:
						{
							if( 0 == vfs_util_dir_foreach(path_temp,proc,p) )
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

				rt = proc(path_temp,0,p);
				switch(rt)
				{
				case VFS_FOREACH_BREAK:
					goto FIND_BREAK;
                case VFS_FOREACH_PROC_ERROR:
                    goto LB_ERROR;
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

VFS_CHAR* vfs_util_str_toupper(VFS_CHAR* str)
{
    VFS_CHAR* p;

    if( !str )
        return NULL;

    p = str;
    while( p && *p != 0 )
    {
        *p = toupper(*p);
        ++p;
    }

    return str;
}

VFS_CHAR* vfs_util_str_tolower(VFS_CHAR* str )
{
    VFS_CHAR* p;

    if( !str )
        return NULL;

    p = str;
    while( p && *p != 0 )
    {
        *p = tolower(*p);
        ++p;
    }

    return str;
}

VFS_CHAR* vfs_util_path_checkfix(VFS_CHAR* path )
{
	VFS_CHAR*p;
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


VFS_CHAR* vfs_util_path_clone(VFS_CHAR*out_path,const VFS_CHAR*path )
{
	if( !out_path || !path )
		return NULL;
	if( strlen(path) <= 0 )
		return NULL;

	strcpy(out_path,path);
	vfs_util_path_checkfix(out_path);
    return out_path;
}

VFS_CHAR* vfs_util_path_append(VFS_CHAR* path ,const VFS_CHAR* append )
{
	const VFS_CHAR *p ;
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

    vfs_util_path_checkfix(path);
	
    return path;
}

VFS_CHAR* vfs_util_path_join(VFS_CHAR* path ,const VFS_CHAR* join )
{
	if( !path || !join)
		return NULL;

	strcat(path,join);
    
    vfs_util_path_checkfix(path);

	return vfs_util_path_checkfix(path);
}

VFS_CHAR* vfs_util_path_combine(VFS_CHAR* path ,const VFS_CHAR* dir ,const VFS_CHAR* append )
{
	if( !path || !dir || !append )
		return NULL;

	if( !vfs_util_path_clone(path,dir) )
		return NULL;

	return vfs_util_path_append( path,append );
}

VFS_CHAR* vfs_util_path_add_backslash(VFS_CHAR* path )
{
	VFS_INT32 len;
	if( !path )
		return NULL;

	vfs_util_path_remove_backslash(path);

	len = strlen(path);
	if( len <= 0 )
		return path;

	strcat(path,"/");
	return path;

}

VFS_CHAR* vfs_util_path_remove_backslash(VFS_CHAR* path )
{
	VFS_INT32 len;
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

VFS_CHAR* vfs_util_path_remove_filename(VFS_CHAR* path )
{
    VFS_INT32 len;
    if( !path )
        return path;

    len = strlen(path);
    if( len <= 0 )
        return path;

    while( len > 0   )
    {
        --len;
        if( path[len] == '/' || path[len] == '\\' )
            break;

        path[len] = 0;
    }

	return path;
}

VFS_CHAR* vfs_util_path_remove_extension(VFS_CHAR* path )
{
    VFS_INT32 len;
    if( !path )
        return path;

    len = strlen(path);
    if( len <= 0 )
        return path;

    while( len > 0   )
    {
        --len;
        if( path[len] == '/' || path[len] == '\\' )
            break;

        if( path[len] == '.' )
        {
            path[len] = 0;
            break;
        }
    }

	return path;
}

const VFS_CHAR* vfs_util_path_get_filename(const VFS_CHAR* path )
{
    VFS_INT32 len;
    VFS_INT32 cursor;
    const VFS_CHAR* p = NULL;

    if( !path )
        return p;

    len = strlen(path);
    if( len <= 0 )
        return p;

    cursor = len;

    while( cursor > 0   )
    {
        if( path[cursor-1] == '/' || path[cursor-1] == '\\' && cursor < len )
        {
            p = &path[cursor];
            break;
        }

        --cursor;
    }

    return path;
}


const VFS_CHAR* vfs_util_path_get_extension(const VFS_CHAR* path )
{
    VFS_INT32 len;
    VFS_INT32 cursor;
    const VFS_CHAR* p = NULL;

    if( !path )
        return path;

    len = strlen(path);
    if( len <= 0 )
        return p;

    cursor = len;


    while( cursor > 0   )
    {
        if( path[cursor-1] == '/' || path[cursor-1] == '\\' )
            break;

        if( path[cursor-1] == '.' && cursor < len )
        {
            p = &path[cursor];
            break;
        }

        --cursor;
    }

    return p;
}

