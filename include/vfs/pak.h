#ifndef _VFS_PAK_H_
#define _VFS_PAK_H_

#include "common.h"

/*
 * header DEFINE
 * */
typedef struct pak_header_s
{
	var32			_M_flag;
	var32			_M_version;
	var32			_M_count;
	uvar64			_M_offset;
}pak_header;

static const var32 pak_header_size = sizeof(var32)+ 
									 sizeof(var32)+ 
									 sizeof(var32)+
									 sizeof(uvar64);

/* 
 * iteminfo DEFINE
 * */
typedef struct pak_iteminfo_s
{
	uvar64			_M_offset;
	
	uvar64			_M_size;
	uvar32			_M_crc32;
	
	var8			_M_compress_type;
	uvar64			_M_compress_size;
	uvar32			_M_compress_crc32;

	char			_M_filename[VFS_MAX_FILENAME+1];
}pak_iteminfo;

/* 
 * pak DEFINE
 * */
typedef struct pak_s
{
	char			_M_filename[VFS_MAX_FILENAME+1];
	pak_header		_M_header;
	pak_iteminfo*	_M_iteminfos;
}pak;


/* 
 * pak API
 * */
VFS_EXTERN pak*				pak_open( const char* );
VFS_EXTERN void				pak_close( pak* );

VFS_EXTERN var32			pak_item_sort_cmp(const void*,const void*);
VFS_EXTERN var32			pak_item_search_cmp(const void*,const void*);

VFS_EXTERN var32			pak_item_getcount( pak* );
VFS_EXTERN pak_iteminfo*	pak_item_getinfo( pak*, var32 );

VFS_EXTERN var32			pak_item_locate( pak*, const char*);
VFS_EXTERN VFS_BOOL			pak_item_unpack_index( pak*, var32, void*, uvar64);
VFS_EXTERN VFS_BOOL			pak_item_unpack_filename( pak*, const char*, void*, uvar64) ;

#endif/*_VFS_PAK_H_*/
