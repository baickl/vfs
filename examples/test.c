#include "pak/pak.h"


const char* media = "../media/test.pak";


int main(int argc,char* argv[])
{

	int i;
	pak *_pak;

	int itemcount;
	pak_iteminfo*iteminfo;

	_pak = pak_open(media);
	if( _pak )
		return -1;

	itemcount = pak_item_getcount(_pak);

	for( i = 0; i<itemcount; ++i )
	{
		iteminfo = pak_item_getinfo(_pak,i);
		printf("pak[%d]:\nname=%s\noffset=%d\nsize=%d\ncrc32=%d\nct-%d\ncs=%d\ncc=%d\n\n",
				i,
				iteminfo->_M_filename,
				iteminfo->_M_offset,
				iteminfo->_M_size,
				iteminfo->_M_crc32,
				iteminfo->_M_compress_type,
				iteminfo->_M_compress_size,
				iteminfo->_M_compress_crc32);
	}

	pak_close(_pak);

}


