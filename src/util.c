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

	strcpy(find_full,path);
	
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
			strcpy(path_temp,path);
			strcat(path_temp,"/");
			strcat(path_temp,entry->d_name);

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
			strcpy(path_temp,path);
			strcat(path_temp,"/");
			strcat(path_temp,entry->d_name);

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

	/*文件句柄  */
	long hFile=0;  

	/*文件信息  */
	struct _finddata_t fileinfo; 

	strcpy(find_full,path);
	strcat(find_full,"*");

	if((hFile=_findfirst(find_full,&fileinfo)) != -1)
	{  

		do
		{
			/*如果是目录,迭代之*/
			/*如果不是,加入列表*/
			if((fileinfo.attrib &_A_SUBDIR)) 
			{  
				if(strcmp(fileinfo.name,".")!= 0 && strcmp(fileinfo.name,"..") != 0 ) 
				{
					memset(path_temp,0,sizeof(path_temp));
					strcpy(path_temp,path);
					strcat(path_temp,"\\");
					strcat(path_temp,fileinfo.name);
					
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
				strcpy(path_temp,path);
				strcat(path_temp,"\\");
				strcat(path_temp,fileinfo.name);

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



char* vfs_util_path_assign(char* path )
{
	return path;
}

char* vfs_util_path_append(char* path ,char* append )
{
	return path;
}

char* vfs_util_path_combine(char* path ,const char* dir ,const char* append )
{
	return path;
}

char* vfs_util_path_add_backslash(char* path )
{
	return path;
}

char* vfs_util_path_remove_backslash(char* path )
{
	return path;
}

char* vfs_util_path_remove_filename(char* path )
{
	return path;
}

char* vfs_util_path_remove_extension(char* path )
{

}
