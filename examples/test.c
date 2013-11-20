/***********************************************************************************
 * Copyright (c) 2013, baickl(baickl@gmail.com)
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 * 
 * * Neither the name of the {organization} nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***********************************************************************************/
#include "vfs/pak.h"
#include <stdio.h>


static FILE* sfopen(const char* filename,const char* mode)
{
#ifndef _WIN32
	return fopen(filename,mode);
#else
	FILE* fp = NULL;
	var32 err;

	err = fopen_s(&fp,filename,mode);
	if( err == 0 )
	{
		return fp;
	}
	else
	{
		return NULL;
	}
#endif
}

const char* media = "../media/test.pak";


int pak_item_saveas(pak* _pak,const char *_file,const char *_saveas)
{
	pak_iteminfo* item;

	int index = -1;
	void* buf = NULL;
	FILE* fp = NULL;

	index  = pak_item_locate(_pak,_file);
	if( index < 0 )
		goto ERROR;

	item = pak_item_getinfo(_pak,index);
	if( !item)
		goto ERROR;

	if( item->_M_size <= 0 )
		goto ERROR;

	buf = (void*)malloc((size_t)item->_M_size);
	if( !pak_item_unpack_filename(_pak,_file,buf,item->_M_size))
		goto ERROR;

	fp = fopen(_saveas,"wb+");
	if( !fp )
		goto ERROR;

	if( fwrite(buf,1,(size_t)item->_M_size,fp) != item->_M_size )
		goto ERROR;

	VFS_SAFE_FREE(buf);
	VFS_SAFE_FCLOSE(fp);

	return 1;

ERROR:
	VFS_SAFE_FREE(buf);
	VFS_SAFE_FCLOSE(fp);

	return 0;
}

int main(int argc,char* argv[])
{

	int i;
	pak *_pak;

	int itemcount;
	pak_iteminfo *iteminfo;

	char filename[VFS_MAX_FILENAME];

	_pak = pak_open(media);
	if( !_pak )
		return -1;

	itemcount = pak_item_getcount(_pak);

	for( i = 0; i<itemcount; ++i )
	{
		iteminfo = pak_item_getinfo(_pak,i);
		printf("pak[%d]:\nname=%s\noffset="I64FMTD"\nsize="I64FMTD"\ncrc32=%d\nct-%d\ncs="I64FMTD"\ncc=%d\n\n",
				i,
				iteminfo->_M_filename,
				iteminfo->_M_offset,
				iteminfo->_M_size,
				iteminfo->_M_crc32,
				iteminfo->_M_compress_type,
				iteminfo->_M_compress_size,
				iteminfo->_M_compress_crc32);

		sprintf(filename,"file_%d",i);
		if(pak_item_saveas(_pak,iteminfo->_M_filename,filename))
			printf("pak save file:%s to %s\n",iteminfo->_M_filename,filename);
	}

	pak_close(_pak);

}


