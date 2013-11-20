#include "vfs/pak.h"
#include "vfs/util.h"
#include "vfs/common.h"
#include <stdio.h>
#include <memory.h>

int pak_item_sort_cmp(const void*a,const void*b)
{

	pak_iteminfo* _a;
	pak_iteminfo* _b;
	
	_a = (pak_iteminfo*)a;
	_b = (pak_iteminfo*)b;

	return stricmp(_a->_M_filename,_b->_M_filename);
}

int pak_item_search_cmp(const void*key,const void*item)
{

	char*_key;
	pak_iteminfo* _item;

	_key  = (const char*)key;
	_item = (const pak_iteminfo*)item;
	return stricmp(_key,_item->_M_filename);
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

	if( header._M_version > VFS_VERSION )
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

		if( filenamelen <= 0 || filenamelen >= VFS_MAX_FILENAME )
			goto ERROR;

		memset(iteminfos[i]._M_filename,0,sizeof(iteminfos[i]._M_filename));
		if( fread(iteminfos[i]._M_filename,1,filenamelen,fp) != filenamelen )
			goto ERROR;
	}

	VFS_SAFE_FCLOSE(fp);


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
	VFS_SAFE_FCLOSE(fp);
	VFS_SAFE_FREE(iteminfos);
	return NULL;
}

void pak_close(pak* _pak)
{
	if( !_pak )
		return;

	if( _pak->_M_iteminfos)
		VFS_SAFE_FREE(_pak->_M_iteminfos);

	VFS_SAFE_FREE(_pak);
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

	vfs_util_path_checkfix(_file);
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

	unsigned int crc32;

	if( !_pak || !_buf )
		return VFS_FALSE;

	iteminfo = pak_item_getinfo(_pak,_index);
	if( !iteminfo )
		return VFS_FALSE;

	if( iteminfo->_M_size > _bufsize )
		return VFS_FALSE;

	/* 
	 * 打开文件尝试读取数据
	 * */

	fp = fopen(_pak->_M_filename,"rb");
	if( !fp )
		return VFS_FALSE;

	if( fseek(fp,iteminfo->_M_offset,SEEK_SET) != 0)
		goto ERROR;

	if( iteminfo->_M_compress_type == VFS_COMPRESS_NONE)
	{
		if( fread(_buf,1,iteminfo->_M_size,fp) != iteminfo->_M_size)
			goto ERROR;

		VFS_SAFE_FCLOSE(fp);
		
		/*
		 * 校验数据是否正确
		 * */
		crc32 = vfs_util_calc_crc32(_buf,iteminfo->_M_size);
		if( crc32 != iteminfo->_M_crc32 )
			goto ERROR;

		return iteminfo->_M_size;
	}
	else if( iteminfo->_M_compress_type == VFS_COMPRESS_BZIP2 )
	{

		compress_buf = malloc(iteminfo->_M_compress_size);
		if( !compress_buf )
			goto ERROR;

		if( fread(compress_buf,1,iteminfo->_M_compress_size,fp) != iteminfo->_M_compress_size)
			goto ERROR;

		if( vfs_util_calc_crc32(compress_buf,iteminfo->_M_compress_size) != iteminfo->_M_compress_crc32)
			goto ERROR;

		if( vfs_util_decompress( iteminfo->_M_compress_type,
								 compress_buf,iteminfo->_M_compress_size,
								 _buf,_bufsize) != iteminfo->_M_size  )
			goto ERROR;

		VFS_SAFE_FREE(compress_buf);
		VFS_SAFE_FCLOSE(fp);

		if( vfs_util_calc_crc32(_buf,iteminfo->_M_size) != iteminfo->_M_crc32) 
			goto ERROR;

		return VFS_TRUE;
	}
	
ERROR:
	VFS_SAFE_FREE(compress_buf);
	VFS_SAFE_FCLOSE(fp);

	return VFS_FALSE;
}

int pak_item_unpack_filename(pak*_pak,const char*_file,void*_buf,int _bufsize)
{
	int index;
	index = pak_item_locate(_pak,_file);
	if( index < 0 )
		return VFS_FALSE;

	return pak_item_unpack_index(_pak,index,_buf,_bufsize);
}
