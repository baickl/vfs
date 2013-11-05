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
	int			_M_offset;
	int			_M_size;
	int			_M_crc32;
	char		_M_filename[MAX_FILENAME+1];
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

int				pak_get_itemcount(pak*);
pak_iteminfo*	pak_get_iteminfo( pak*,int );

int				pak_locate_item(pak*,const char*);
bool			pak_extract_item(pak*,int,void*,int);


#endif//_PAK_H_
