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
#include <pak.h>
#include <vfs/vfs.h>
#include <stdio.h>

const char* media = "../media/src.pak";

int pak_item_saveas(pak* _pak,const char *_file,const char *_saveas);

VFS_INT32 pak_item_foreach_for_save(pak* _pak,char*filename,pak_iteminfo* iteminfo,int index,void*p )
{
    char _filename[VFS_MAX_FILENAME];
    printf("pak[%d]:\nname=%s\noffset=" I64FMTU "\nsize=" I64FMTU "\ncrc32=%d\nct=%s\ncs=" I64FMTU "\n",
            index,
            filename,
            iteminfo->_M_offset,
            iteminfo->_M_size,
            iteminfo->_M_crc32,
            iteminfo->_M_compress_plugin,
            iteminfo->_M_compress_size);

    sprintf(_filename,"file_%d",index);
    if(pak_item_saveas(_pak,filename,_filename))
        printf("pak save file:%s to %s\n\n",filename,_filename);

    return VFS_FOREACH_CONTINUE;
}


int pak_item_saveas(pak* _pak,const char *_file,const char *_saveas)
{
	pak_iteminfo* item;

	int index = -1;
	void* buf = NULL;
	FILE* fp = NULL;

	item  = pak_item_locate(_pak,_file);
	if( !item)
		goto ERROR;

	if( item->_M_size <= 0 )
		goto ERROR;

	buf = (void*)vfs_pool_malloc((size_t)item->_M_size);
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

int main(void)
{
	pak *_pak;

	int itemcount;

    if( VFS_TRUE != vfs_create(VFS_SDK_VERSION,".") )
        return -1;

	_pak = pak_open(media,NULL);
	if( !_pak )
		return -1;

	itemcount = pak_item_get_count(_pak);
	if( itemcount > 0 )
        pak_item_foreach(_pak,pak_item_foreach_for_save,NULL);
	
    pak_close(_pak);


    vfs_destroy();

    return -1;
}


