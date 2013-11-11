#include "pak/pak.h"
#include "crc32/crc32.h"
#include "bzip2/bzlib.h"
#include <stdio.h>
#include <memory.h>


int pak_item_sort_cmp(const void*a,const void*b)
{

	pak_iteminfo* _a;
	pak_iteminfo* _b;
	
	_a = (pak_iteminfo*)a;
	_b = (pak_iteminfo*)b;

	return strcmp(_a->_M_filename,_b->_M_filename);
}

int pak_item_search_cmp(const void*key,const void*item)
{

	char*_key;
	pak_iteminfo* _item;

	_key  = (const char*)key;
	_item = (const pak_iteminfo*)item;
	return strcmp(_key,_item->_M_filename);
}

pak* pak_open(const char* _pakfile)
{
	int i;
	pak *_pak;
	FILE *fp;
	pak_header header;
	pak_iteminfo*iteminfos;
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
	{
		fclose(fp);
		return NULL;
	}

	if( header._M_flag != 'pakx')
	{
		fclose(fp);
		return NULL;
	}

	if( header._M_version > PAK_VERSION )
	{
		fclose(fp);
		return NULL;
	}

	if( header._M_count <= 0 )
	{
		fclose(fp);
		return NULL;
	}
	

	/* 
	 * 提取文件信息
	 * */

	iteminfos = (pak_iteminfo*)malloc(header._M_count*sizeof(pak_iteminfo));
	if( !iteminfos )
	{
		fclose(fp);
		return NULL;
	}

	for( i = 0; i<header._M_count; ++i )
	{


		if( fread(&iteminfos[i]._M_offset,1,sizeof(iteminfos[i]._M_offset),fp) != sizeof(iteminfos[i]._M_offset))
		{
			fclose(fp);
			free(iteminfos);
			return NULL;
		}

		iteminfos[i]._M_offset += header._M_offset; 

		if( fread(&iteminfos[i]._M_size,1,sizeof(iteminfos[i]._M_size),fp) != sizeof(iteminfos[i]._M_size))
		{
			fclose(fp);
			free(iteminfos);
			return NULL;
		}

		if( fread(&iteminfos[i]._M_crc32,1,sizeof(iteminfos[i]._M_crc32),fp) != sizeof(iteminfos[i]._M_size))
		{
			fclose(fp);
			free(iteminfos);
			return NULL;
		}

		if( fread(&iteminfos[i]._M_compress_type,1,sizeof(iteminfos[i]._M_compress_type),fp) != sizeof(iteminfos[i]._M_compress_type))
		{
			fclose(fp);
			free(iteminfos);
			return NULL;
		}

		if( fread(&iteminfos[i]._M_compress_size,1,sizeof(iteminfos[i]._M_compress_size),fp) != sizeof(iteminfos[i]._M_compress_size))
		{
			fclose(fp);
			free(iteminfos);
			return NULL;
		}


		if( fread(&iteminfos[i]._M_compress_crc32,1,sizeof(iteminfos[i]._M_compress_crc32),fp) != sizeof(iteminfos[i]._M_compress_crc32))
		{
			fclose(fp);
			free(iteminfos);
			return NULL;
		}

		filenamelen = 0;
		if( fread(&filenamelen,1,sizeof(filenamelen),fp) != sizeof(filenamelen))
		{
			fclose(fp);
			free(iteminfos);
			return NULL;
		}

		if( filenamelen <= 0 || filenamelen >= PAK_MAX_FILENAME )
		{
			fclose(fp);
			free(iteminfos);
			return NULL;
		}

		memset(iteminfos[i]._M_filename,0,sizeof(iteminfos[i]._M_filename));
		if( fread(iteminfos[i]._M_filename,1,filenamelen,fp) != filenamelen )
		{
			fclose(fp);
			free(iteminfos);
			return NULL;
		}

		printf("iteminfo[%d]:\nname=%s\nsize=%d\ncrc32=%d\nct=%d\ncs=%d\ncc=%d\n\n",
				i,
				iteminfos[i]._M_filename,
				iteminfos[i]._M_size,
				iteminfos[i]._M_crc32,
				iteminfos[i]._M_compress_type,
				iteminfos[i]._M_compress_size,
				iteminfos[i]._M_compress_crc32);


	}


	if( fp )
	{
		fclose(fp);
		fp = NULL;
	}


	/* 
	 * 读取成功，组织文件包
	 * */
	_pak = (pak*)malloc(sizeof(pak));
	if( !_pak )
	{
		free(iteminfos);
		return NULL;
	}
	memset(_pak->_M_filename,0,sizeof(_pak->_M_filename));
	memset(&_pak->_M_header,0,sizeof(_pak->_M_header));
	strcpy(_pak->_M_filename,_pakfile);
	memcpy(&_pak->_M_header,&header,sizeof(header));
	_pak->_M_iteminfos = iteminfos;

	printf("successed:open_pak count=%d,offset=%d\n",
			_pak->_M_header._M_count,
			_pak->_M_header._M_offset);

	/*
	 * 排序
	 * */
	qsort((void*)_pak->_M_iteminfos,_pak->_M_header._M_count,sizeof(pak_iteminfo),pak_item_sort_cmp);
	return _pak;
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

	pak_iteminfo* iteminfo;
	FILE* fp;

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

	if( fseek(fp,iteminfo->_M_offset,SEEK_SET) != iteminfo->_M_offset)
	{
		fclose(fp);
		return 0;
	}

	if( iteminfo->_M_compress_type == PAK_COMPRESS_NONE)
	{
		if( fread(_buf,1,iteminfo->_M_size,fp) != iteminfo->_M_size)
		{
			fclose(fp);
			return 0;
		}

		fclose(fp);
		fp = 0;

		/*
		 * 校验数据是否正确
		 * */
		unsigned int crc32 = pak_util_calc_crc32(_buf,iteminfo->_M_size);
		if( crc32 != iteminfo->_M_crc32 )
		{
			return 0;
		}

		return iteminfo->_M_size;
	}
	else if( iteminfo->_M_compress_type == PAK_COMPRESS_BZIP2 )
	{

		void* compress_buf = malloc(iteminfo->_M_compress_size);
		if( !compress_buf )
		{
			fclose(fp);
			return 0;
		}

		if( fread(compress_buf,1,iteminfo->_M_compress_size,fp) != iteminfo->_M_compress_size)
		{
			free(compress_buf);
			fclose(fp);
			return 0;
		}

		if( pak_util_calc_crc32(compress_buf,iteminfo->_M_compress_size) != iteminfo->_M_compress_crc32)
		{
			free(compress_buf);
			fclose(fp);
			return 0;
		}

		if( pak_util_decompress( iteminfo->_M_compress_type,
								 compress_buf,iteminfo->_M_compress_size,
								 _buf,_bufsize) != iteminfo->_M_size  )
		{
			free(compress_buf);
			fclose(fp);
			return 0;
		}

		free(compress_buf);
		fclose(fp);

		if( pak_util_calc_crc32(_buf,iteminfo->_M_size) != iteminfo->_M_crc32) 
		{
			return 0;
		}

		
		return iteminfo->_M_size;
	}
	else
	{
		fclose(fp);
		return 0;
	}
}

int pak_item_unpack_filename(pak*_pak,const char*_file,void*_buf,int _bufsize)
{
	int index;
	index = pak_item_locate(_pak,_file);
	if( index < 0 )
		return 0;

	return pak_item_unpack_index(_pak,index,_buf,_bufsize);
}
