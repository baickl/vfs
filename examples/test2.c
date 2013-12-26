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
#include <vfs/vfs.h>
#include <vfs/stream.h>
#include <stdio.h>

int main( void )
{

	vfs_stream *vf;
    vfs_stream *out;

	size_t realsize ;
	char buf[VFS_MAX_FILENAME+1];

	if( VFS_TRUE != vfs_create(VFS_SDK_VERSION,".."))
		goto ERROR;

	if( VFS_TRUE != vfs_add_archive("media/src.pak",NULL))
    {
        printf("open pak error \n");
		goto ERROR;
    }

	vf = vfs_file_open("mediA/src/bzip2/randtable.c");
	if( !vf )
    {
        printf("vfs_file_open failed\n");
		goto ERROR;
    }

    out = vfs_stream_new();

	while( !vf->stream_eof(vf) )
	{
		realsize = vf->stream_read(vf,buf,1,(size_t)VFS_MAX_FILENAME);
		if( realsize > 0 )
		{
			buf[realsize] = 0;
			printf(buf);

            if( out->stream_write(out,buf,1,realsize) != realsize )
            {
                printf("Write Error!\n");
            }
		}
	}

    /* ±£´æÒ»ÏÂ */
    out->stream_save(out,"./randtable.c");

    vfs_stream_delete(out);
	vfs_stream_delete(vf);

	vfs_destroy();
	return 0;

ERROR:
	vfs_destroy();
	return -1;

}
