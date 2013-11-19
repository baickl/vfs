#include "vfs/util.h"
#include "crc32/crc32.h"
#include "bzip2/bzlib.h"
#include <stdio.h>
#include <memory.h>
#include <sys/types.h>
#include <dirent.h>


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
		r = BZ2_bzBuffToBuffCompress(dst,&compressed_size,src,srcsize,9,3,30);
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
		r = BZ2_bzBuffToBuffDecompress(dst,&uncompressed_size,src,srcsize,0,2);
		if( r != BZ_OK )
			return 0;
		return uncompressed_size;
	case VFS_COMPRESS_NONE:
	default:
		return 0;
	}
}

int vfs_util_dir_foreach(const char* path,dir_foreach_item_proc proc)
{
	DIR* dir;
	struct dirent *entry = NULL;

	char find_full[VFS_MAX_FILENAME+1];
	char path_temp[VFS_MAX_FILENAME+1];

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
			if( strcmp(entry->d_name,".")  == 0 ||
				strcmp(entry->d_name,"..") == 0 )
				continue;
			memset(path_temp,0,sizeof(path_temp));
			strcpy(path_temp,path);
			strcat(path_temp,"/");
			strcat(path_temp,entry->d_name);

			if( proc )
				proc(path_temp,1);

			if( 0 == vfs_util_dir_foreach(path_temp,proc) )
				goto ERROR;
		}		
		else
		{
			memset(path_temp,0,sizeof(path_temp));
			strcpy(path_temp,path);
			strcat(path_temp,"/");
			strcat(path_temp,entry->d_name);

			if( proc )
				proc(path_temp,0);
		}
	}

	closedir(dir);
	return 1;

ERROR:
	closedir(dir);
	return 0;

}


