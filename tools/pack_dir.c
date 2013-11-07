#include "pak/pak.h"
#include <stdio.h>

static pak *g_pak = NULL;

int pak_begin( const char *path )
{
	pak* _pak = (pak*)malloc(sizeof(pak));
	if( !_pak)
		return 0;

	g_pak = _pak;
	return 1;
}

int dir_collect_fileinfo( const char *path )
{
	/* TODO: */
}

int dir_pack( const char *path ) 
{
	/* TODO: */
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

	const char* path = argv[1];
	
	if( 0 == pak_begin(path) )
		return -1;
	
	if( 0 == dir_collect_fileinfo(path))
		return -1;

	if( 0 == dir_pack(path) )
		return -1;
	
	pak_end(path);

	return 0;
}
