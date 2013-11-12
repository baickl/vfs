#include "pak/pak.h"
#include "crc32/crc32.h"
#include "bzip2/bzlib.h"
#include <stdio.h>
#include <memory.h>
#include <sys/types.h>
#include <dirent.h>

int pak_item_sort_cmp(const void*a,const void*b)
{

	pak_iteminfo* _a;
	pak_iteminfo* _b;
	
	_a = (pak_iteminfo*)a;
	_b = (pak_iteminfo*)b;

	return strcasecmp(_a->_M_filename,_b->_M_filename);
}

int pak_item_search_cmp(const void*key,const void*item)
{

	char*_key;
	pak_iteminfo* _item;

	_key  = (const char*)key;
	_item = (const pak_iteminfo*)item;
	return strcasecmp(_key,_item->_M_filename);
}

pak* pak_open(const char* _pakfile)
{
	int i;
	pak *_pak = NULL;
	FILE *fp = NULL;
	pak_header header;
	pak_iteminfo*iteminfos = NULL;
	int filenamelen;

	/*
	 * 打开文件
	 * */
	fp = fopen(_pakfile,"rb");
	if( !fp )
		return NULL;

	/* 
	 * 校验HEADER头
	 * */
	if( fread(&header,1,sizeof(header),fp) != sizeof(header))
		goto ERROR;

	if( header._M_flag != 'pakx')
		goto ERROR;

	if( header._M_version > PAK_VERSION )
		goto ERROR;

	if( header._M_count <= 0 )
		goto ERROR;	

	/* 
	 * 提取文件信息
	 * */

	iteminfos = (pak_iteminfo*)malloc(header._M_count*sizeof(pak_iteminfo));
	if( !iteminfos )
		goto ERROR;	

	for( i = 0; i<header._M_count; ++i )
	{

		if( fread(&iteminfos[i]._M_offset,1,sizeof(iteminfos[i]._M_offset),fp) != sizeof(iteminfos[i]._M_offset))
			goto ERROR;
		iteminfos[i]._M_offset += header._M_offset; 
		
		if( fread(&iteminfos[i]._M_size,1,sizeof(iteminfos[i]._M_size),fp) != sizeof(iteminfos[i]._M_size))
			goto ERROR;

		if( fread(&iteminfos[i]._M_crc32,1,sizeof(iteminfos[i]._M_crc32),fp) != sizeof(iteminfos[i]._M_size))
			goto ERROR;	

		if( fread(&iteminfos[i]._M_compress_type,1,sizeof(iteminfos[i]._M_compress_type),fp) != sizeof(iteminfos[i]._M_compress_type))
			goto ERROR;

		if( fread(&iteminfos[i]._M_compress_size,1,sizeof(iteminfos[i]._M_compress_size),fp) != sizeof(iteminfos[i]._M_compress_size))
			goto ERROR;	

		if( fread(&iteminfos[i]._M_compress_crc32,1,sizeof(iteminfos[i]._M_compress_crc32),fp) != sizeof(iteminfos[i]._M_compress_crc32))
			goto ERROR;

		filenamelen = 0;
		if( fread(&filenamelen,1,sizeof(filenamelen),fp) != sizeof(filenamelen))
			goto ERROR;

		if( filenamelen <= 0 || filenamelen >= PAK_MAX_FILENAME )
			goto ERROR;

		memset(iteminfos[i]._M_filename,0,sizeof(iteminfos[i]._M_filename));
		if( fread(iteminfos[i]._M_filename,1,filenamelen,fp) != filenamelen )
			goto ERROR;
	}

	PAK_SAFE_FCLOSE(fp);


	/* 
	 * 读取成功，组织文件包
	 * */
	_pak = (pak*)malloc(sizeof(pak));
	if( !_pak )
		goto ERROR;
	
	memset(_pak->_M_filename,0,sizeof(_pak->_M_filename));
	memset(&_pak->_M_header,0,sizeof(_pak->_M_header));
	strcpy(_pak->_M_filename,_pakfile);
	memcpy(&_pak->_M_header,&header,sizeof(header));
	_pak->_M_iteminfos = iteminfos;

	/*
	 * 排序
	 * */
	qsort((void*)_pak->_M_iteminfos,_pak->_M_header._M_count,sizeof(pak_iteminfo),pak_item_sort_cmp);
	return _pak;

ERROR:
	PAK_SAFE_FCLOSE(fp);
	PAK_SAFE_FREE(iteminfos);
	return NULL;
}

void pak_close(pak* _pak)
{
	if( !_pak )
		return;

	if( _pak->_M_iteminfos)
		free(_pak->_M_iteminfos);

	free(_pak);
}


unsigned int pak_util_calc_crc32(void*buf,int size)
{
	return calc_crc32(buf,size);
}

int pak_util_compress_bound( int compresstype, int srclen )
{
	switch(compresstype)
	{
	case PAK_COMPRESS_BZIP2:
		return (int)(srclen*1.01+600);

	case PAK_COMPRESS_NONE:
	default:
		return srclen;
	}
}

int pak_util_compress(int compresstype, const void*src,int srcsize,void*dst,int dstsize)
{
	int r;
	int compressed_size = dstsize;

	switch(compresstype)
	{
	case PAK_COMPRESS_BZIP2:
		r = BZ2_bzBuffToBuffCompress(dst,&compressed_size,src,srcsize,9,3,30);
		if( r != BZ_OK)
			return 0;
		return compressed_size;

	case PAK_COMPRESS_NONE:
	default:
		return 0;
	}
}

int pak_util_decompress(int compresstype,const void*src,int srcsize,void*dst,int dstsize)
{
	int r;
	int uncompressed_size = dstsize;
	switch(compresstype)
	{
	case PAK_COMPRESS_BZIP2:
		r = BZ2_bzBuffToBuffDecompress(dst,&uncompressed_size,src,srcsize,0,2);
		if( r != BZ_OK )
			return 0;
		return uncompressed_size;
	case PAK_COMPRESS_NONE:
	default:
		return 0;
	}
}

int pak_util_dir_foreach(const char* path,dir_foreach_item_proc proc)
{
	DIR* dir;
	struct dirent *entry = NULL;

	char find_full[PAK_MAX_FILENAME+1];
	char path_temp[PAK_MAX_FILENAME+1];

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

			if( 0 == pak_util_dir_foreach(path_temp,proc) )
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

int pak_item_getcount(pak* _pak)
{
	if( !_pak)
		return 0;

	return _pak->_M_header._M_count;
}

pak_iteminfo* pak_item_getinfo(pak*_pak,int _index )
{
	if( !_pak)
		return NULL;

	if( _index < 0 || _index >= _pak->_M_header._M_count )
		return NULL;

	return &_pak->_M_iteminfos[_index];
}

int  pak_item_locate(pak*_pak,const char* _file)
{
	int ret = -1;
	pak_iteminfo* iteminfo=NULL;

	if( !_pak || !_file)
		return -1;

	iteminfo = (pak_iteminfo*)bsearch(_file,_pak->_M_iteminfos,_pak->_M_header._M_count,sizeof(pak_iteminfo),pak_item_search_cmp);
	if( !iteminfo )
		return -1;

	return (iteminfo-_pak->_M_iteminfos);

}

int pak_item_unpack_index( pak* _pak,int _index,void *_buf,int _bufsize)
{
	pak_iteminfo* iteminfo = NULL;
	FILE* fp = NULL;
	void* compress_buf = NULL;

	if( !_pak || !_buf )
		return 0;

	iteminfo = pak_item_getinfo(_pak,_index);
	if( !iteminfo )
		return 0;

	if( iteminfo->_M_size > _bufsize )
		return 0;

	/* 
	 * 打开文件尝试读取数据
	 * */

	fp = fopen(_pak->_M_filename,"rb");
	if( !fp )
		return 0;

	if( fseek(fp,iteminfo->_M_offset,SEEK_SET) != 0)
		goto ERROR;

	if( iteminfo->_M_compress_type == PAK_COMPRESS_NONE)
	{
		if( fread(_buf,1,iteminfo->_M_size,fp) != iteminfo->_M_size)
			goto ERROR;

		PAK_SAFE_FCLOSE(fp);
		
		/*
		 * 校验数据是否正确
		 * */
		unsigned int crc32 = pak_util_calc_crc32(_buf,iteminfo->_M_size);
		if( crc32 != iteminfo->_M_crc32 )
			return 0;

		return iteminfo->_M_size;
	}
	else if( iteminfo->_M_compress_type == PAK_COMPRESS_BZIP2 )
	{

		compress_buf = malloc(iteminfo->_M_compress_size);
		if( !compress_buf )
			goto ERROR;

		if( fread(compress_buf,1,iteminfo->_M_compress_size,fp) != iteminfo->_M_compress_size)
			goto ERROR;

		if( pak_util_calc_crc32(compress_buf,iteminfo->_M_compress_size) != iteminfo->_M_compress_crc32)
			goto ERROR;

		if( pak_util_decompress( iteminfo->_M_compress_type,
								 compress_buf,iteminfo->_M_compress_size,
								 _buf,_bufsize) != iteminfo->_M_size  )
			goto ERROR;

		PAK_SAFE_FREE(compress_buf);
		PAK_SAFE_FCLOSE(fp);

		if( pak_util_calc_crc32(_buf,iteminfo->_M_size) != iteminfo->_M_crc32) 
			return 0;
		
		return iteminfo->_M_size;
	}
	
ERROR:
	PAK_SAFE_FREE(compress_buf);
	PAK_SAFE_FCLOSE(fp);

	return 0;
}

int pak_item_unpack_filename(pak*_pak,const char*_file,void*_buf,int _bufsize)
{
	int index;
	index = pak_item_locate(_pak,_file);
	if( index < 0 )
		return 0;

	return pak_item_unpack_index(_pak,index,_buf,_bufsize);
}
