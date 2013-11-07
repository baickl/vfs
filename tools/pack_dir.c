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
	FILE* fp;

	FILE* fp_head;
	FILE* fp_data;

	for( i = 0; i<g_pak->_M_header._M_count; ++i )
	{
		
	}
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
		return -1;
	
	if( 0 == dir_collect_fileinfo(path))
		return -1;

	if( 0 == dir_pack(path) )
		return -1;
	
	pak_end(path);

	return 0;
}
