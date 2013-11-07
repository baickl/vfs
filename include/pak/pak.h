#ifndef _PAK_H_
#define _PAK_H_


/* 
 * MACRO DEFINE
 * */

#define PAK_VERSION			0x0001

#define PAK_MAX_FILENAME	512

#define PAK_COMPRESS_NONE	0x0000
#define PAK_COMPRESS_BZIP2	0x0001

/*
 * header DEFINE
 * */
typedef struct pak_header_s
{
	int _M_flag;
	int _M_version;
	int _M_count;
	int _M_offset;
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

	char			_M_filename[PAK_MAX_FILENAME+1];
}pak_iteminfo;

/* 
 * pak DEFINE
 * */
typedef struct pak_s
{
	char			_M_filename[PAK_MAX_FILENAME+1];
	pak_header		_M_header;
	pak_iteminfo*	_M_iteminfos;
}pak;


/* 
 * pak API
 * */
pak*			pak_open( const char* );
void			pak_close( pak* );

unsigned int	pak_util_calc_crc32( void*, int );

int				pak_util_compress_bound( int ,int );
int				pak_util_compress( int,  const void*, int,void*, int );
int				pak_util_decompress( int, const void*, int,void*, int );

int				pak_item_getcount( pak* );
pak_iteminfo*	pak_item_getinfo( pak*, int );

int				pak_item_locate( pak*, const char*);
int				pak_item_unpack_index( pak*, int, void*, int);
int				pak_item_unpack_filename( pak*, const char*, void*, int) ;

#endif/*_PAK_H_*/
