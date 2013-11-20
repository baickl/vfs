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
#include "vfs/pak.h"
#include "vfs/util.h"
#include "vfs/common.h"
#include <stdio.h>
#include <memory.h>

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

var32 pak_item_sort_cmp(const void*a,const void*b)
{

	pak_iteminfo* _a;
	pak_iteminfo* _b;
	
	_a = (pak_iteminfo*)a;
	_b = (pak_iteminfo*)b;

	return stricmp(_a->_M_filename,_b->_M_filename);
}

var32 pak_item_search_cmp(const void*key,const void*item)
{

	char*_key;
	pak_iteminfo* _item;

	_key  = (char*)key;
	_item = (pak_iteminfo*)item;
	return stricmp(_key,_item->_M_filename);
}

pak* pak_open(const char* _pakfile)
{
	var32 i;
	pak *_pak = NULL;
	FILE *fp = NULL;
	pak_header header;
	pak_iteminfo*iteminfos = NULL;
	uvar16 filenamelen;

	/*
	 * 打开文件
	 * */
	fp = sfopen(_pakfile,"rb");
	if( !fp )
		return NULL;

	/* 
	 * 校验HEADER头
	 * */
	if( fread(&header._M_flag,1,sizeof(header._M_flag),fp) != sizeof(header._M_flag))
		goto ERROR;

	if( header._M_flag != MAKE_CC_ID('p','a','k','x'))
		goto ERROR;

	if( fread(&header._M_version,1,sizeof(header._M_version),fp) != sizeof(header._M_version))
		goto ERROR;

	if( header._M_version > VFS_VERSION )
		goto ERROR;

	if( fread(&header._M_count,1,sizeof(header._M_count),fp) != sizeof(header._M_count))
		goto ERROR;

	if( header._M_count <= 0 )
		goto ERROR;	

	if( fread(&header._M_offset,1,sizeof(header._M_offset),fp) != sizeof(header._M_offset))
		goto ERROR;

	if( header._M_offset <= 0 )
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

		if( fread(&iteminfos[i]._M_crc32,1,sizeof(iteminfos[i]._M_crc32),fp) != sizeof(iteminfos[i]._M_crc32))
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



var32 pak_item_getcount(pak* _pak)
{
	if( !_pak)
		return 0;

	return _pak->_M_header._M_count;
}

pak_iteminfo* pak_item_getinfo(pak*_pak,var32 _index )
{
	if( !_pak)
		return NULL;

	if( _index < 0 || _index >= _pak->_M_header._M_count )
		return NULL;

	return &_pak->_M_iteminfos[_index];
}

var32  pak_item_locate(pak*_pak,const char* _file)
{
	var32 ret = -1;
	pak_iteminfo* iteminfo=NULL;

	if( !_pak || !_file)
		return -1;

	vfs_util_path_checkfix(_file);
	iteminfo = (pak_iteminfo*)bsearch(_file,_pak->_M_iteminfos,_pak->_M_header._M_count,sizeof(pak_iteminfo),pak_item_search_cmp);
	if( !iteminfo )
		return -1;

	return (iteminfo-_pak->_M_iteminfos);

}

var32 pak_item_unpack_index( pak* _pak,var32 _index,void *_buf,uvar64 _bufsize)
{
	pak_iteminfo* iteminfo = NULL;
	FILE* fp = NULL;
	void* compress_buf = NULL;

	uvar32 crc32;

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

	fp = sfopen(_pak->_M_filename,"rb");
	if( !fp )
		return VFS_FALSE;

	if( VFS_FSEEK(fp,iteminfo->_M_offset,SEEK_SET) != 0)
		goto ERROR;

	if( iteminfo->_M_compress_type == VFS_COMPRESS_NONE)
	{
		if( fread(_buf,1,(size_t)iteminfo->_M_size,fp) != iteminfo->_M_size)
			goto ERROR;

		VFS_SAFE_FCLOSE(fp);
		
		/*
		 * 校验数据是否正确
		 * */
		crc32 = vfs_util_calc_crc32(_buf,(var32)iteminfo->_M_size);
		if( crc32 != iteminfo->_M_crc32 )
			goto ERROR;

		return VFS_TRUE;
	}
	else if( iteminfo->_M_compress_type == VFS_COMPRESS_BZIP2 )
	{

		compress_buf = (void*)malloc(iteminfo->_M_compress_size);
		if( !compress_buf )
			goto ERROR;

		if( fread(compress_buf,1,(size_t)iteminfo->_M_compress_size,fp) != iteminfo->_M_compress_size)
			goto ERROR;

		if( vfs_util_calc_crc32(compress_buf,(var32)iteminfo->_M_compress_size) != iteminfo->_M_compress_crc32)
			goto ERROR;

		if( vfs_util_decompress( iteminfo->_M_compress_type,
								 compress_buf,iteminfo->_M_compress_size,
								 _buf,_bufsize) != iteminfo->_M_size  )
			goto ERROR;

		VFS_SAFE_FREE(compress_buf);
		VFS_SAFE_FCLOSE(fp);

		if( vfs_util_calc_crc32(_buf,(var32)iteminfo->_M_size) != iteminfo->_M_crc32) 
			goto ERROR;

		return VFS_TRUE;
	}
	
ERROR:
	VFS_SAFE_FREE(compress_buf);
	VFS_SAFE_FCLOSE(fp);

	return VFS_FALSE;
}

VFS_BOOL pak_item_unpack_filename(pak*_pak,const char*_file,void*_buf,uvar64 _bufsize)
{
	var32 index;
	index = pak_item_locate(_pak,_file);
	if( index < 0 )
		return VFS_FALSE;

	return pak_item_unpack_index(_pak,index,_buf,_bufsize);
}
