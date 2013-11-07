#include "pak/pak.h"
#include <stdio.h>
#include <io.h>

static pak *g_pak = NULL;
static int  g_maxcount = 0;
static char g_dir[PAK_MAX_FILENAMELEN+1];

int pak_begin( const char *path )
{
	pak* _pak = (pak*)malloc(sizeof(pak));
	if( !_pak)
		return 0;

	g_pak = _pak;
	g_pak->_M_header._M_count = 0;
	g_maxcount = 0;
	memset(g_dir,0,sizeof(g_dir));

	return 1;
}

int pak_additeminfo( const char* path )
{
	pak_iteminfo* iteminfo;

	if( g_pak->_M_header._M_count >= g_maxcount )
	{
		if( g_pak->_M_header._M_count == 0 )
			g_maxcount = 256;
		else
			g_maxcount += g_maxcount;

		_pak->_M_iteminfos = (pak_iteminfo*)realloc(_pak->_M_iteminfos,g_maxcount*sizeof(pak_iteminfo));
		if( !_pak->_M_iteminfos )
			return 0;
	}

	iteminfo = g_pak->_M_iteminfos[g_pak->_M_header._M_count++];

	memset(iteminfo,0,sizeof(pak_iteminfo));
	strcpy(iteminfo->_M_filename,path);
	return 0;

}

int dir_collect_fileinfo( const char *path )
{
	long handle;
	struct _finddata_t fd;

	char find_full[PAK_MAXFILENAME+1];
	char path_temp[PAK_MAXFILENAME+1];

	strcpy(find_full,path);
	strcat(find_full,"*");
	
	handle = _findfirst(find_full,&fd);
	if( -1 == handle )
		return 0;

	while( !_findnext(handle,&fd) )
	{

		if( fd.attrib & _A_SUBDIR )
		{
			if( strcmp(fd.name,".")  == 0 ||
				strcmp(fd.name,"..") == 0 )
				continue;

			memset(path_temp,0,sizeof(path_temp));
			strcpy(path_temp,path);
			strcat(path_temp,fd.name);

			if( 0 == dir_collect_fileinfo(path_temp) )
			{
				_findclose(handle);
				return 0;
			}
		}
		else
		{
			memset(path_temp,0,sizeof(path_temp));
			strcpy(path_temp,path);
			strcat(path_temp,fd.name);
			
			if( 0 == pak_additeminf(path_temp) )
			{
				_findclose(handle);
				return 0;
			}
		}
	}

	_findclose(handle);
	return 1;
}

int dir_pack( const char *path ) 
{

	int i;

	pak_iteminfo* iteminfo = NULL;

	FILE* fp = NULL;

	FILE* fp_head = NULL;
	FILE* fp_iteminfo = NULL;
	FILE* fp_data = NULL;

	void* buf = NULL ;

	int	compress_buf_size = 0 ;
	void* compress_buf = NULL ;

	int compress_result = 0;

	fp_head = fopen("pak_header.tmp","wb+");
	if( !fp_head )
	{
		return 0;
	}

	fp_iteminfo = fopen("pak_iteminfo.tmp","wb+");
	if( !fp_iteminfo )
	{
		fclose(fp_head);
		return 0;
	}

	fp_data = fopen("pak_data.tmp","wb+");
	if( !fp_data )
	{
		fclose(fp_iteminfo);
		fclose(fp_head);
		return 0;
	}

	for( i = 0; i<g_pak->_M_header._M_count; ++i )
	{
		iteminfo = &g_pak->_M_iteminfos[i];

		fp = fopen(iteminfo->_M_filename);
		if( !fp )
		{
			fclose(fp_data);
			fclose(fp_iteminfo);
			fclose(fp_head);
			return 0;
		}

		fseek(fp,0,SEEK_END);
		iteminfo->_M_size = ftell(fp);
		fseek(fp,0,SEEK_SET);

		iteminfo->_M_offset = ftell(fp_data);

		if( iteminfo->_M_size <= 0 )
		{
			iteminfo->_M_size = 0;
			iteminfo->_M_crc32 = 0;

			iteminfo->_M_compress_type = PAK_COMPRESS_NONE;
			iteminfo->_M_compress_size = 0;
			iteminfo->_M_compress_crc32 = 0;
		}
		else
		{
			buf = malloc(iteminfo->_M_size);
			if(fread(buf,iteminfo->_M_size,1,fp) != iteminfo->_M_size)
			{
				free(buf);
				fclose(fp);
				fclose(fp_data);
				fclose(fp_iteminfo);
				fclose(fp_head);
				return 0;
			}

			fclose(fp);

			iteminfo->_M_crc32 = pak_util_calc_crc32(buf,iteminfo->_M_size);

			compress_buf_size = pak_util_compress_bound(PAK_COMPRESS_BZIP2,iteminfo->_M_size);
			compress_buf = malloc(compress_buf_size);

			compress_result = pak_util_compress(buf,iteminfo->_M_size,compress_buf,compress_buf_size);
			if(compress_result >= iteminfo->_M_size)
			{
				iteminfo->_M_compress_type = PAK_COMPRESS_NONE;
				iteminfo->_M_compress_size = 0;
				iteminfo->_M_compress_crc32 = 0;

				free(compress_buf);
				compress_buf = 0;
				compress_buf_size = 0;

				if( fwrite(buf,iteminfo->_M_size,1,fp_data) != iteminfo->_M_size)
				{
					free(buf);
					free(compress_buf);

					fclose(fp);
					fclose(fp_data);
					fclose(fp_iteminfo);
					fclose(fp_head);

					return 0;
				}

				free(buf);
				buf = 0;

			}
			else
			{
				iteminfo->_M_compress_type = PAK_COMPRESS_BZIP2;
				iteminfo->_M_compress_size = compress_result;
				iteminfo->_M_-compress_crc32 = pak_util_calc_crc32(compress_buf,iteminfo->_M_compress_size);

				free(buf);
				buf = 0;

				if( fwrite(compress_buf,iteminfo->_M_compress_size,1,fp_data) != iteminfo->_M_compress_size)
				{
					free(compress_buf);
					compress_buf = 0;
					compress_buf_size = 0;

					fclose(fp);
					fclose(fp_data);
					fclose(fp_iteminfo);
					fclose(fp_head);

					return 0;
				}

				free(compress_buf);
				compress_buf = 0;
				compress_buf_size = 0;
			}
		}
	}


	/*
	 * 写入head
	 * */


	/*
	 * 组合文件
	 * */
}


int pak_end( const char *path )
{
	if( g_pak )
	{
		if ( g_pak->_M_iteminfo )
		{
			free(g_pak->_M_iteminfo);
			g_pak->_M_iteminfo;
		}

		free(g_pak);
		g_pak = NULL;
	}

}

int main( int argc,char *argv[] )
{
	if(argc != 2 )
		return -1;

	const char* path = argv[1];
	memset(g_dir,0,sizeof(g_dir));
	strcpy(g_dir,path);

	if( 0 == pak_begin(path) )
	{
		pak_end(path);
		return -1;
	}
	
	if( 0 == dir_collect_fileinfo(path))
	{
		pak_end(path);
		return -1;
	}

	if( 0 == dir_pack(path) )
	{
		pak_end(path);
		return -1;
	}
	
	pak_end(path);
	return 0;
}
