#ifndef _PAK_H_
#define _PAK_H_


#define MAX_FILENAME = 512;

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
	char			_M_filename[MAX_FILENAME+1];
}pak_iteminfo;

/* 
 * pak DEFINE
 * */
typedef struct pak_s
{
	char			_M_filename[MAX_FILENAME+1];
	pak_header		_M_header;
	pak_iteminfo*	_M_iteminfos;
}pak;


/* 
 * pak API
 * */
pak*			pak_open(const char*);
void			pak_close(pak*);

int				pak_item_getcount(pak*);
pak_iteminfo*	pak_item_getinfo( pak*,int );

int				pak_item_locate(pak*,const char*);
bool			pak_item_unpack_index(pak*,int,void*,int);
bool			pak_item_unpack_filename(pak*,const char*,void*,int);


#endif//_PAK_H_
