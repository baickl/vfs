#ifndef _VFS_PAK_H_
#define _VFS_PAK_H_

#include "common.h"


typedef int (*dir_foreach_item_proc)(const char*fullpath,int dir);

/*
 * header DEFINE
 * */
typedef struct pak_header_s
{
	int				_M_flag;
	int				_M_version;
	int				_M_count;
	int				_M_offset;
}pak_header;

/* 
 * iteminfo DEFINE
 * */
typedef struct pak_iteminfo_s
{
	int				_M_offset;
	
	int				_M_size;
	unsigned int	_M_crc32;
	
	int				_M_compress_type;
	int				_M_compress_size;
	int				_M_compress_crc32;

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

VFS_EXTERN unsigned int		pak_util_calc_crc32( void*, int );
VFS_EXTERN int				pak_util_compress_bound( int ,int );
VFS_EXTERN int				pak_util_compress( int,  const void*, int,void*, int );
VFS_EXTERN int				pak_util_decompress( int, const void*, int,void*, int );
VFS_EXTERN int				pak_util_dir_foreach(const char*,dir_foreach_item_proc);

VFS_EXTERN int				pak_item_sort_cmp(const void*,const void*);
VFS_EXTERN int				pak_item_search_cmp(const void*,const void*);

VFS_EXTERN int				pak_item_getcount( pak* );
VFS_EXTERN pak_iteminfo*	pak_item_getinfo( pak*, int );

VFS_EXTERN int				pak_item_locate( pak*, const char*);
VFS_EXTERN int				pak_item_unpack_index( pak*, int, void*, int);
VFS_EXTERN int				pak_item_unpack_filename( pak*, const char*, void*, int) ;

#endif/*_VFS_PAK_H_*/
