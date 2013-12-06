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
#include <vfs/util.h>
#include <stdio.h>

#include <string.h>
#include <sys/types.h>
#ifdef __linux__
#include <dirent.h>
#else
#include <io.h>  
#include <direct.h>
#endif 

static pak *g_pak = NULL;
static var32  g_maxcount = 0;
static char g_dir[VFS_MAX_FILENAME+1];
static var32  g_dirlen = 0;

char g_file_header[VFS_MAX_FILENAME+1]={0};
char g_file_iteminfo[VFS_MAX_FILENAME+1]={0};
char g_file_data[VFS_MAX_FILENAME+1]={0};



static FILE* sfopen(const char* filename,const char* mode)
{
#ifdef __linux__
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

uvar64 file_getlen(FILE*fp)
{
	uvar64 curpos;
	uvar64 flen = 0;

	if( !fp )
		return 0;
	curpos = VFS_FTELL(fp);
	VFS_FSEEK(fp,0,SEEK_END);
	flen = VFS_FTELL(fp);
	VFS_FSEEK(fp,curpos,SEEK_SET);
	return flen;
}

VFS_BOOL pak_begin( const char *path )
{
	pak* _pak = (pak*)malloc(sizeof(pak));
	if( !_pak)
		return VFS_FALSE;

	memset(_pak,0,sizeof(pak));

	g_pak = _pak;
	g_pak->_M_header._M_flag = MAKE_CC_ID('p','a','k','x');
	g_pak->_M_header._M_version = PAK_VERSION;
	g_pak->_M_header._M_count = 0;
    g_pak->_M_ht_iteminfos = create_hashtable(256,pak_item_hashcode,pak_item_equalkeys,pak_item_key_free);
    if( !g_pak->_M_ht_iteminfos )
        return VFS_FALSE;

	return VFS_TRUE;
}

VFS_BOOL pak_additeminfo( const char* filepath )
{

    var32 filenamelen;
	pak_iteminfo* iteminfo;
   
    char* filename;

    if( !g_pak || !g_pak->_M_ht_iteminfos  )
        return VFS_FALSE;

    iteminfo = (pak_iteminfo*)malloc(sizeof(pak_iteminfo));
    if( !iteminfo )return VFS_FALSE;
	memset(iteminfo,0,sizeof(pak_iteminfo));
    
    filenamelen = strlen(filepath);
    filename = (char*)malloc(filenamelen+1);
    if( filename == NULL )
    {
        VFS_SAFE_FREE(iteminfo);
        return VFS_FALSE;
    }

    memset(filename,0,filenamelen+1);
    strcpy(filename,filepath);
    vfs_util_path_checkfix(filename);
    vfs_util_str_tolower(filename);

    if( ! pak_item_insert(g_pak->_M_ht_iteminfos,
                          filename,
                          iteminfo ))
    {
        VFS_SAFE_FREE(filename);
        VFS_SAFE_FREE(iteminfo);
        return VFS_FALSE;
    }

    g_pak->_M_header._M_count ++;

	return VFS_TRUE;

}

var32 dir_collect_fileinfo_proc(const char*fullpath,var32 dir,void* p)
{
	const char* strfile;

	strfile = fullpath + g_dirlen;
	if( !dir )
	{
		strfile = fullpath + g_dirlen;
		pak_additeminfo(strfile);

		printf("\tfind file:%s\n",strfile);
	}
	else
	{
		printf("\nenter dir:%s\n",strfile);
	}

	return VFS_FOREACH_CONTINUE;
}


VFS_BOOL dir_collect_fileinfo( const char *_path )
{
	if( vfs_util_dir_foreach(_path,dir_collect_fileinfo_proc,NULL) )
        return VFS_TRUE;
    else
        return VFS_FALSE;
}


VFS_BOOL fwrite_data(FILE*fp,void*buf,var32 bufsize)
{
	if( !fp )
		return VFS_FALSE;

	if( !buf || bufsize <= 0  )
		return VFS_FALSE;

	if( fwrite(buf,1,bufsize,fp) != bufsize)
		return VFS_FALSE;

	return VFS_TRUE;
}

var32 pak_item_foreach_for_write(pak* _pak,char*filename,pak_iteminfo* iteminfo,int index,void*p )
{
    FILE*fp;
    uvar16 len;

    fp = (FILE*)p;
    if( !fp || !filename)
        return VFS_FOREACH_PROC_ERROR;
    
    if( !VFS_CHECK_FWRITE(fp,&iteminfo->_M_offset,sizeof(iteminfo->_M_offset)))
        return VFS_FOREACH_PROC_ERROR;

    if( !VFS_CHECK_FWRITE(fp,&iteminfo->_M_size,sizeof(iteminfo->_M_size)))
        return VFS_FOREACH_PROC_ERROR;

    if( !VFS_CHECK_FWRITE(fp,&iteminfo->_M_crc32,sizeof(iteminfo->_M_crc32)))
        return VFS_FOREACH_PROC_ERROR;

    if( !VFS_CHECK_FWRITE(fp,&iteminfo->_M_compress_type,sizeof(iteminfo->_M_compress_type)))
        return VFS_FOREACH_PROC_ERROR;

    if( !VFS_CHECK_FWRITE(fp,&iteminfo->_M_compress_size,sizeof(iteminfo->_M_compress_size)))
        return VFS_FOREACH_PROC_ERROR;

    if( !VFS_CHECK_FWRITE(fp,&iteminfo->_M_compress_crc32,sizeof(iteminfo->_M_compress_crc32)))
        return VFS_FOREACH_PROC_ERROR;

    len = strlen(filename);
    if( !VFS_CHECK_FWRITE(fp,&len,sizeof(len)))
        return VFS_FOREACH_PROC_ERROR;

    if( !VFS_CHECK_FWRITE(fp,filename,len))
        return VFS_FOREACH_PROC_ERROR;

    return VFS_FOREACH_CONTINUE;
}

VFS_BOOL fwrite_iteminfos(FILE* fp)
{
	if( !fp )
		goto LBL_FI_ERROR;

	if( VFS_FSEEK(fp,0,SEEK_SET) != 0 )
		goto LBL_FI_ERROR;

	if( g_pak->_M_header._M_count <= 0 )
		goto LBL_FI_ERROR;
    
    if( VFS_TRUE != pak_item_foreach(g_pak,pak_item_foreach_for_write,(void*)fp))
        goto LBL_FI_ERROR;
	

	return VFS_TRUE;

LBL_FI_ERROR:
	return VFS_FALSE;
}

VFS_BOOL fwrite_header(FILE*fp)
{
	if( !fp )
		return VFS_FALSE;

	if( VFS_FSEEK(fp,0,SEEK_SET) != 0 )
		goto LBL_FH_ERROR;

	if( !VFS_CHECK_FWRITE(fp,&g_pak->_M_header._M_flag,sizeof(g_pak->_M_header._M_flag)))
		goto LBL_FH_ERROR;

	if( !VFS_CHECK_FWRITE(fp,&g_pak->_M_header._M_version,sizeof(g_pak->_M_header._M_version)))
		goto LBL_FH_ERROR;

	if( !VFS_CHECK_FWRITE(fp,&g_pak->_M_header._M_count,sizeof(g_pak->_M_header._M_count)))
		goto LBL_FH_ERROR;

	if( !VFS_CHECK_FWRITE(fp,&g_pak->_M_header._M_offset,sizeof(g_pak->_M_header._M_offset)))
		goto LBL_FH_ERROR;

	return VFS_TRUE;
LBL_FH_ERROR:
	return VFS_FALSE;
}


VFS_BOOL pakfile_combine(FILE* fp_header,FILE*fp_iteminfo,FILE* fp_data,const char* outputfile)
{

	FILE *fp = NULL;
	FILE *fp_temp = NULL;

	var32 bufsize = 1024;
	var32 readsize =0;
	char buf[1024];

	if( !fp_header || !fp_iteminfo || !fp_data )
		return VFS_FALSE;

	fp = sfopen(outputfile,"wb+");
	if( !fp )
	{
		printf("error:pakfile_combine create outputfile %s failed\n",outputfile);
		return VFS_FALSE;
	}

	VFS_FSEEK(fp_header,0,SEEK_SET);
	VFS_FSEEK(fp_iteminfo,0,SEEK_SET);
	VFS_FSEEK(fp_data,0,SEEK_SET);

	printf("info:pakfile_combine begin pak header size=" I64FMTU "\n",file_getlen(fp_header));
	fp_temp = fp_header;
	while(!feof(fp_temp))
	{
		readsize = fread(buf,1,bufsize,fp_temp);
		if(readsize>0)
		{
			if( !VFS_CHECK_FWRITE(fp,buf,readsize))
				goto LB_ERROR;
		}
	}

	printf("info:pakfile_combine begin pak iteminfo size=" I64FMTU "\n",file_getlen(fp_iteminfo));
	fp_temp = fp_iteminfo;
	while(!feof(fp_temp))
	{
		readsize = fread(buf,1,bufsize,fp_temp);
		if(readsize>0)
		{
			if( !VFS_CHECK_FWRITE(fp,buf,readsize))
				goto LB_ERROR;
		}
	}

	printf("info:pakfile_combine begin pak data size="I64FMTU"\n",file_getlen(fp_data));
	fp_temp = fp_data;
	while(!feof(fp_temp))
	{
		readsize = fread(buf,1,bufsize,fp_temp);
		if(readsize>0)
		{
			if( !VFS_CHECK_FWRITE(fp,buf,readsize))
				goto LB_ERROR;
		}
	}
	
	VFS_SAFE_FCLOSE(fp);
	return VFS_TRUE;

LB_ERROR:
	VFS_SAFE_FCLOSE(fp);
	return VFS_FALSE;

}

var32 pak_item_foreach_for_pack(pak* _pak,char *filename,pak_iteminfo* iteminfo,int index,void*p )
{
    FILE*fp;
    void* buf = NULL ;
	uvar64	compress_buf_size = 0 ;
	void* compress_buf = NULL ;
	uvar64 compress_result = 0;
	uvar64 tmp = 0;
    char filetemp[VFS_MAX_FILENAME+1];

    FILE* fp_data = (FILE*)p;
    if( !fp_data )
        return VFS_FOREACH_PROC_ERROR;

    memset(filetemp,0,sizeof(filetemp));
    if( g_dirlen > 0 )
        vfs_util_path_combine(filetemp,g_dir,filename);
    else
        vfs_util_path_clone(filetemp,filename);

    fp = sfopen(filetemp,"rb");
    if( !fp )
    {
        printf("error:dir_pack open file %s failed\n",filename);
        goto LBL_DP_ERROR;
    }

    iteminfo->_M_size = file_getlen(fp);
    iteminfo->_M_offset = VFS_FTELL(fp_data);

    if( iteminfo->_M_size <= 0 )
    {
        VFS_SAFE_FCLOSE(fp);
        iteminfo->_M_size = 0;
        iteminfo->_M_crc32 = 0;

        iteminfo->_M_compress_type = VFS_COMPRESS_NONE;
        iteminfo->_M_compress_size = 0;
        iteminfo->_M_compress_crc32 = 0;

        printf("warning:dir_pack empty file %s\n",filename);
    }
    else
    {
        buf = (void*)malloc(iteminfo->_M_size);
        tmp = fread(buf,1,(size_t)iteminfo->_M_size,fp);
        if( tmp != iteminfo->_M_size)
        {
            printf("error:dir_pack read file %s size=" I64FMTU " readsize=" I64FMTU " fpos=" I64FMTU " failed\n",
                    filename,
                    iteminfo->_M_size,
                    tmp,
                    VFS_FTELL(fp));
            goto LBL_DP_ERROR;
        }

        VFS_SAFE_FCLOSE(fp);

        iteminfo->_M_crc32 = vfs_util_calc_crc32(buf,(var32)iteminfo->_M_size);

        compress_buf_size = vfs_util_compress_bound(VFS_COMPRESS_BZIP2,(var32)iteminfo->_M_size);
        compress_buf = (void*)malloc(compress_buf_size);

        compress_result = vfs_util_compress(VFS_COMPRESS_BZIP2,buf,iteminfo->_M_size,compress_buf,compress_buf_size);
        if(compress_result == 0 || compress_result >= iteminfo->_M_size)
        {
            iteminfo->_M_compress_type = VFS_COMPRESS_NONE;
            iteminfo->_M_compress_size = 0;
            iteminfo->_M_compress_crc32 = 0;

            VFS_SAFE_FREE(compress_buf);
            compress_buf_size = 0;

            if( !VFS_CHECK_FWRITE(fp_data,buf,iteminfo->_M_size))
            {
                printf("error:dir_pack pack file[no compress] %s failed\n",filename);
                goto LBL_DP_ERROR;
            }

            VFS_SAFE_FREE(buf);
        }
        else
        {
            iteminfo->_M_compress_type = VFS_COMPRESS_BZIP2;
            iteminfo->_M_compress_size = compress_result;
            iteminfo->_M_compress_crc32 = vfs_util_calc_crc32(compress_buf,(var32)compress_result);

            VFS_SAFE_FREE(buf);

            if( !VFS_CHECK_FWRITE(fp_data,compress_buf,compress_result))
            {
                printf("error:dir_pack pack file[compress] %s failed\n",filename);
                goto LBL_DP_ERROR;
            }

            VFS_SAFE_FREE(compress_buf);
            compress_buf_size = 0;
        }
    }


    printf("\nsuccessed:dir_pack pack file %s OK\n",filename);
    printf("\tfile=%s\n",filename);
    printf("\tsize=" I64FMTU "\n",iteminfo->_M_size);
    printf("\tcrc32=%d\n",iteminfo->_M_crc32);
    printf("\tcompress_type=%d\n",iteminfo->_M_compress_type);
    printf("\tcompress_size=" I64FMTU "\n",iteminfo->_M_compress_size);
    printf("\tcompress_crc32=%d\n",iteminfo->_M_compress_crc32);
    printf("\toffset=" I64FMTU "\n\n",iteminfo->_M_offset);

    return VFS_FOREACH_CONTINUE;

LBL_DP_ERROR:
	VFS_SAFE_FREE(buf);
	VFS_SAFE_FREE(compress_buf);
	VFS_SAFE_FCLOSE(fp);

	return VFS_FOREACH_PROC_ERROR;
}


VFS_BOOL dir_pack( const char *path,const char* output ) 
{
	VFS_BOOL combinefile_result = VFS_FALSE;

	FILE* fp_head = NULL;
	FILE* fp_iteminfo = NULL;
	FILE* fp_data = NULL;

 	remove(g_file_header);
 	remove(g_file_iteminfo);
 	remove(g_file_data);

	fp_head = sfopen(g_file_header,"wb+");
	if( !fp_head )
	{
		printf("error:dir_pack create pak_header.tmp failed\n");
		return VFS_FALSE;
	}

	fp_iteminfo = sfopen(g_file_iteminfo,"wb+");
	if( !fp_iteminfo )
	{
		printf("error:dir_pack create pak_iteminfo.tmp failed\n");
		goto LBL_DP_ERROR;
	}

	fp_data = sfopen(g_file_data,"wb+");
	if( !fp_data )
	{
		printf("error:dir_pack create pak_data.tmp failed\n");
		goto LBL_DP_ERROR;
	}

    if( VFS_FALSE == pak_item_foreach(g_pak,pak_item_foreach_for_pack,fp_data))
    {
		printf("error:dir_pack pack_iteminfos failed\n");
        goto LBL_DP_ERROR;
    }

	if( VFS_FALSE == fwrite_iteminfos(fp_iteminfo) )
	{
		printf("error:dir_pack fwrite_iteminfos failed\n");
		goto LBL_DP_ERROR;
	}

	g_pak->_M_header._M_offset = VFS_FTELL(fp_iteminfo)+pak_header_size;
	if( VFS_FALSE == fwrite_header(fp_head) )
	{
		printf("error:dir_pack fwrite_header failed\n");
		goto LBL_DP_ERROR;
	}

	remove(output);
	combinefile_result = pakfile_combine(fp_head,fp_iteminfo,fp_data,output);

	VFS_SAFE_FCLOSE(fp_data);
	VFS_SAFE_FCLOSE(fp_iteminfo);
	VFS_SAFE_FCLOSE(fp_head);

	return combinefile_result;

LBL_DP_ERROR:
	VFS_SAFE_FCLOSE(fp_data);
	VFS_SAFE_FCLOSE(fp_iteminfo);
	VFS_SAFE_FCLOSE(fp_head);

	return VFS_FALSE;
}

void pak_end( const char *path )
{
	remove(g_file_header);
	remove(g_file_iteminfo);
	remove(g_file_data);

	if( g_pak )
	{
        if( g_pak->_M_ht_iteminfos )
        {
            printf("free hashtable beg \n");
            hashtable_destroy(g_pak->_M_ht_iteminfos);
            g_pak->_M_ht_iteminfos = NULL;
            printf("free hashtable end \n");
        }

		VFS_SAFE_FREE(g_pak);
	}
}



int main( int argc,char *argv[] )
{
	char path[VFS_MAX_FILENAME+1] = {0};
	char outfile[VFS_MAX_FILENAME+1] = {0};

	int i;
	int index;

	if(argc != 2 )
	{
		printf("usage: pack_dir <directory> \n");
		return -1;
	}

	if( !vfs_util_path_clone(path,argv[1]) )
		return -1;

	if( !vfs_util_path_clone(outfile,path) )
		return -1;

	vfs_util_path_remove_backslash(outfile);

	if( !vfs_util_path_join(outfile,".pak"))
		return -1;

	
	
	printf("pack_dir %s %s\n",path,outfile);

	memset(g_dir,0,sizeof(g_dir));
	vfs_util_path_clone(g_dir,path);
	vfs_util_path_remove_backslash(g_dir);


	vfs_util_path_clone(g_file_header,g_dir);
	vfs_util_path_join(g_file_header,".pak_header.tmp");

	vfs_util_path_clone(g_file_iteminfo,g_dir);
	vfs_util_path_join(g_file_iteminfo,".pak_iteminfo.tmp");

	vfs_util_path_clone(g_file_data,g_dir);
	vfs_util_path_join(g_file_data,".pak_data.tmp");


	remove(g_file_header);
	remove(g_file_iteminfo);
	remove(g_file_data);
	remove(outfile);

	
	index = 0;
	g_dirlen = strlen(g_dir);
	for( i = 0; i<g_dirlen; ++i )
	{
		if( g_dir[i] == '/' )
			index = i+1;
	}

	if( index != 0 )
	{
		g_dirlen = index;
		g_dir[g_dirlen] = 0;
	}
	else
	{
		g_dirlen = 0;
	}
	


	if( VFS_FALSE == pak_begin(path) )
	{
		printf("error:pak_begin failed\n ");
		goto LB_ERROR;
	}
	
	if( VFS_FALSE == dir_collect_fileinfo(path))
	{
		printf("error:dir_collect_fileinfo failed \n");
		goto LB_ERROR;
	}


	if( VFS_FALSE == dir_pack(path,outfile) )
	{
		printf("error:dir_pack failed\n");
		goto LB_ERROR;
	}
	
	pak_end(path);
	return 0;

LB_ERROR:
	pak_end(path);
	return -1;
}
