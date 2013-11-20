#include "vfs/util.h"
#include "crc32/crc32.h"
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


unsigned int vfs_util_calc_crc32(void*buf,int size)
{
	return calc_crc32(buf,size);
}

int vfs_util_compress_bound( int compresstype, int srclen )
{
	switch(compresstype)
	{
	case VFS_COMPRESS_BZIP2:
		return (int)(srclen*1.01+600);

	case VFS_COMPRESS_NONE:
	default:
		return srclen;
	}
}

int vfs_util_compress(int compresstype, const void*src,int srcsize,void*dst,int dstsize)
{
	int r;
	int compressed_size = dstsize;

	switch(compresstype)
	{
	case VFS_COMPRESS_BZIP2:
		r = BZ2_bzBuffToBuffCompress((char*)dst,&compressed_size,(char*)src,srcsize,9,3,30);
		if( r != BZ_OK)
			return 0;
		return compressed_size;

	case VFS_COMPRESS_NONE:
	default:
		return 0;
	}
}

int vfs_util_decompress(int compresstype,const void*src,int srcsize,void*dst,int dstsize)
{
	int r;
	int uncompressed_size = dstsize;
	switch(compresstype)
	{
	case VFS_COMPRESS_BZIP2:
		r = BZ2_bzBuffToBuffDecompress((char*)dst,&uncompressed_size,(char*)src,srcsize,0,2);
		if( r != BZ_OK )
			return 0;
		return uncompressed_size;
	case VFS_COMPRESS_NONE:
	default:
		return 0;
	}
}


#ifndef _WIN32
int vfs_util_dir_foreach(const char* path,dir_foreach_item_proc proc)
{
	DIR* dir;
	struct dirent *entry = NULL;

	char find_full[VFS_MAX_FILENAME+1];
	char path_temp[VFS_MAX_FILENAME+1];

	int rt;

	if( !vfs_util_path_clone(find_full,path))
		return 0;
	
	dir = opendir(path);
	if( NULL == dir )
		return 0;

	if( NULL == proc )
		return 0;

	while( (entry=readdir(dir)) != NULL )
	{
		if( entry->d_type & DT_DIR )
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
	return 1;

LB_ERROR:
	closedir(dir);
	return 0;

}

#else

int vfs_util_dir_foreach(const char* path,dir_foreach_item_proc proc)
{
	char find_full[VFS_MAX_FILENAME+1] = {0};
	char path_temp[VFS_MAX_FILENAME+1] = {0};

	int rt;

	long hFile=0;  
	struct _finddata_t fileinfo; 

	if( !vfs_util_path_combine(find_full,path,"*") )
		return 0;

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

	return 1;

LB_ERROR:
	if( hFile )
		_findclose(hFile);  
	return 0;

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
	char *p ;
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
	int len;
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
	int len;
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
