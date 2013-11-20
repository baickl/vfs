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
#include "vfs/util.h"
#include <stdio.h>

#include <string.h>

#include <sys/types.h>
#ifndef _WIN32
#include <dirent.h>
#else
#include <io.h>  
#include <direct.h>
#endif 

static pak *g_pak = NULL;
static var32  g_maxcount = 0;
static char g_dir[VFS_MAX_FILENAME+1];
static var32  g_dirlen = 0;

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
	g_pak->_M_header._M_flag = 'pakx';
	g_pak->_M_header._M_version = VFS_VERSION;
	g_pak->_M_header._M_count = 0;
	g_pak->_M_iteminfos = NULL;

	g_maxcount = 0;

	return VFS_TRUE;
}

VFS_BOOL pak_additeminfo( const char* filepath )
{
	pak_iteminfo* iteminfo;

	if( g_pak->_M_header._M_count >= g_maxcount )
	{
		if( g_pak->_M_header._M_count == 0 )
		{
			g_maxcount = 256;
			g_pak->_M_iteminfos = (pak_iteminfo*)malloc(g_maxcount*sizeof(pak_iteminfo));
			
		}
		else
		{
			g_maxcount += g_maxcount;
			g_pak->_M_iteminfos = (pak_iteminfo*)realloc(g_pak->_M_iteminfos,g_maxcount*sizeof(pak_iteminfo));
		}

		
		if( !g_pak->_M_iteminfos )
			return VFS_FALSE;
	}

	iteminfo = &g_pak->_M_iteminfos[g_pak->_M_header._M_count++];

	memset(iteminfo,0,sizeof(pak_iteminfo));
	strcpy(iteminfo->_M_filename,filepath);
	vfs_util_path_checkfix(iteminfo->_M_filename);
	return VFS_TRUE;

}

var32 dir_collect_fileinfo_proc(const char*fullpath,var32 dir)
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

	return DIR_FOREACH_CONTINUE;
}


VFS_BOOL dir_collect_fileinfo( const char *_path )
{
	return vfs_util_dir_foreach(_path,dir_collect_fileinfo_proc);
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

VFS_BOOL fwrite_iteminfos(FILE* fp)
{
	var32 i;
	uvar16 len;
	pak_iteminfo * iteminfo;

	if( !fp )
		goto LBL_FI_ERROR;

	if( VFS_FSEEK(fp,0,SEEK_SET) != 0 )
		goto LBL_FI_ERROR;

	if( g_pak->_M_header._M_count <= 0 )
		goto LBL_FI_ERROR;

	qsort(g_pak->_M_iteminfos,g_pak->_M_header._M_count,sizeof(pak_iteminfo),pak_item_sort_cmp);

	for( i = 0; i<g_pak->_M_header._M_count; ++i )
	{
		iteminfo = &g_pak->_M_iteminfos[i];

		if( !VFS_CHECK_FWRITE(fp,&iteminfo->_M_offset,sizeof(iteminfo->_M_offset)))
			goto LBL_FI_ERROR; 

		if( !VFS_CHECK_FWRITE(fp,&iteminfo->_M_size,sizeof(iteminfo->_M_size)))
			goto LBL_FI_ERROR; 

		if( !VFS_CHECK_FWRITE(fp,&iteminfo->_M_crc32,sizeof(iteminfo->_M_crc32)))
			goto LBL_FI_ERROR; 

		if( !VFS_CHECK_FWRITE(fp,&iteminfo->_M_compress_type,sizeof(iteminfo->_M_compress_type)))
			goto LBL_FI_ERROR; 

		if( !VFS_CHECK_FWRITE(fp,&iteminfo->_M_compress_size,sizeof(iteminfo->_M_compress_size)))
			goto LBL_FI_ERROR; 

		if( !VFS_CHECK_FWRITE(fp,&iteminfo->_M_compress_crc32,sizeof(iteminfo->_M_compress_crc32)))
			goto LBL_FI_ERROR; 

		len = strlen(iteminfo->_M_filename);
		if( !VFS_CHECK_FWRITE(fp,&len,sizeof(len)))
			goto LBL_FI_ERROR; 

		if( !VFS_CHECK_FWRITE(fp,iteminfo->_M_filename,len))
			goto LBL_FI_ERROR; 
	}

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

	printf("info:pakfile_combine begin pak header size="I64FMTD"\n",file_getlen(fp_header));
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

	printf("info:pakfile_combine begin pak iteminfo size="I64FMTD"\n",file_getlen(fp_iteminfo));
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

	printf("info:pakfile_combine begin pak data size="I64FMTD"\n",file_getlen(fp_data));
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


VFS_BOOL dir_pack( const char *path,const char* output ) 
{
	var32 i;

	pak_iteminfo* iteminfo = NULL;

	FILE* fp = NULL;

	FILE* fp_head = NULL;
	FILE* fp_iteminfo = NULL;
	FILE* fp_data = NULL;

	void* buf = NULL ;

	uvar64	compress_buf_size = 0 ;
	void* compress_buf = NULL ;

	uvar64 compress_result = 0;
	VFS_BOOL combinefile_result = VFS_FALSE;

	var32 tmp = 0;


	char filetemp[VFS_MAX_FILENAME+1];
	

	char file_header[VFS_MAX_FILENAME+1]={0};
	char file_iteminfo[VFS_MAX_FILENAME+1]={0};
	char file_data[VFS_MAX_FILENAME+1]={0};

	strcpy(file_header,g_dir);
	strcat(file_header,".pak_header.tmp");

	strcpy(file_iteminfo,g_dir);
	strcat(file_iteminfo,".pak_iteminfo.tmp");

	strcpy(file_data,g_dir);
	strcat(file_data,".pak_data.tmp");


	
	
 	remove(file_header);
 	remove(file_iteminfo);
 	remove(file_data);

	fp_head = sfopen(file_header,"wb+");
	if( !fp_head )
	{
		printf("error:dir_pack create pak_header.tmp failed\n");
		return VFS_FALSE;
	}

	fp_iteminfo = sfopen(file_iteminfo,"wb+");
	if( !fp_iteminfo )
	{
		printf("error:dir_pack create pak_iteminfo.tmp failed\n");
		goto LBL_DP_ERROR;
	}

	fp_data = sfopen(file_data,"wb+");
	if( !fp_data )
	{
		printf("error:dir_pack create pak_data.tmp failed\n");
		goto LBL_DP_ERROR;
	}

	for( i = 0; i<g_pak->_M_header._M_count; ++i )
	{
		iteminfo = &g_pak->_M_iteminfos[i];


		memset(filetemp,0,sizeof(filetemp));
		if( g_dirlen > 0 )
			vfs_util_path_combine(filetemp,g_dir,iteminfo->_M_filename);
		else
			vfs_util_path_clone(filetemp,iteminfo->_M_filename);

		fp = sfopen(filetemp,"rb");
		if( !fp )
		{
			printf("error:dir_pack open file %s failed\n",iteminfo->_M_filename);
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

			printf("warning:dir_pack empty file %s\n",iteminfo->_M_filename);
		}
		else
		{
			buf = (void*)malloc(iteminfo->_M_size);
			tmp = fread(buf,1,(size_t)iteminfo->_M_size,fp);
			if( tmp != iteminfo->_M_size)
			{
				printf("error:dir_pack read file %s size=%d readsize=%d fpos=" I64FMTD " failed\n",
						iteminfo->_M_filename,
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
					printf("error:dir_pack pack file[no compress] %s failed\n",iteminfo->_M_filename);
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
					printf("error:dir_pack pack file[compress] %s failed\n",iteminfo->_M_filename);
					goto LBL_DP_ERROR;
				}

				VFS_SAFE_FREE(compress_buf);
				compress_buf_size = 0;
			}
		}


		printf("\nsuccessed:dir_pack pack file %s OK\n",iteminfo->_M_filename);
		printf("\tfile=%s\n",iteminfo->_M_filename);
		printf("\tsize="I64FMTD"\n",iteminfo->_M_size);
		printf("\tcrc32=%d\n",iteminfo->_M_crc32);
		printf("\tcompress_type=%d\n",iteminfo->_M_compress_type);
		printf("\tcompress_size="I64FMTD"\n",iteminfo->_M_compress_size);
		printf("\tcompress_crc32=%d\n",iteminfo->_M_compress_crc32);
		printf("\toffset="I64FMTD"\n\n",iteminfo->_M_offset);

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
	VFS_SAFE_FREE(buf);
	VFS_SAFE_FREE(compress_buf);
	VFS_SAFE_FCLOSE(fp);
	VFS_SAFE_FCLOSE(fp_data);
	VFS_SAFE_FCLOSE(fp_iteminfo);
	VFS_SAFE_FCLOSE(fp_head);

	return VFS_FALSE;
}

void pak_end( const char *path )
{
	char file_header[VFS_MAX_FILENAME+1]={0};
	char file_iteminfo[VFS_MAX_FILENAME+1]={0};
	char file_data[VFS_MAX_FILENAME+1]={0};
	strcpy(file_header,g_dir);
	strcat(file_header,".pak_header.tmp");

	strcpy(file_iteminfo,g_dir);
	strcat(file_iteminfo,".pak_iteminfo.tmp");

	strcpy(file_data,g_dir);
	strcat(file_data,".pak_data.tmp");

	remove(file_header);
	remove(file_iteminfo);
	remove(file_data);

	if( g_pak )
	{
		VFS_SAFE_FREE(g_pak->_M_iteminfos);
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

	if( !vfs_util_path_join(outfile,".pak"))
		return -1;

	remove(outfile);
	printf("pack_dir %s %s\n",path,outfile);

	memset(g_dir,0,sizeof(g_dir));
	vfs_util_path_clone(g_dir,path);
	vfs_util_path_remove_backslash(g_dir);
	
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
