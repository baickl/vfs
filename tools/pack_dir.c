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
static int  g_maxcount = 0;
static char g_dir[VFS_MAX_FILENAME+1];
static int  g_dirlen = 0;

FILE* sfopen(const char* filename,const char* mode)
{
#ifndef _WIN32
	return fopen(filename,mode);
#else
	FILE* fp = NULL;
	int err;
	
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

int file_getlen(FILE*fp)
{
	int curpos;
	int flen = 0;

	if( !fp )
		return 0;

	curpos = ftell(fp);
	fseek(fp,0,SEEK_END);
	flen = ftell(fp);
	fseek(fp,curpos,SEEK_SET);
	return flen;

}

int pak_begin( const char *path )
{
	pak* _pak = (pak*)malloc(sizeof(pak));
	if( !_pak)
		return 0;

	memset(_pak,0,sizeof(pak));

	g_pak = _pak;
	g_pak->_M_header._M_flag = 'pakx';
	g_pak->_M_header._M_version = VFS_VERSION;
	g_pak->_M_header._M_count = 0;
	g_pak->_M_iteminfos = NULL;

	g_maxcount = 0;

	return 1;
}

int pak_additeminfo( const char* filepath )
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
			return 0;
	}

	iteminfo = &g_pak->_M_iteminfos[g_pak->_M_header._M_count++];

	memset(iteminfo,0,sizeof(pak_iteminfo));
	strcpy(iteminfo->_M_filename,filepath);
	vfs_util_path_checkfix(iteminfo->_M_filename);
	return 1;

}

int dir_collect_fileinfo_proc(const char*fullpath,int dir)
{
	const char* strfile;

	strfile = fullpath + g_dirlen + 1;
	if( !dir )
	{
		strfile = fullpath + g_dirlen + 1;
		pak_additeminfo(strfile);

		printf("\tfind file:%s\n",strfile);
	}
	else
	{
		printf("\nenter dir:%s\n",strfile);
	}

	return DIR_FOREACH_CONTINUE;
}


int dir_collect_fileinfo( const char *_path )
{
	return vfs_util_dir_foreach(_path,dir_collect_fileinfo_proc);
}


int fwrite_data(FILE*fp,void*buf,int bufsize)
{
	if( !fp )
		return 0;

	if( !buf || bufsize <= 0  )
		return 0;

	if( fwrite(buf,1,bufsize,fp) != bufsize)
		return 0;

	return 1;
}

int fwrite_iteminfos(FILE* fp)
{
	int i;
	int len;
	pak_iteminfo * iteminfo;

	if( !fp )
		goto LBL_FI_ERROR;

	if( fseek(fp,0,SEEK_SET) != 0 )
		goto LBL_FI_ERROR;

	if( g_pak->_M_header._M_count <= 0 )
		goto LBL_FI_ERROR;

	qsort(g_pak->_M_iteminfos,g_pak->_M_header._M_count,sizeof(pak_iteminfo),pak_item_sort_cmp);

	for( i = 0; i<g_pak->_M_header._M_count; ++i )
	{
		iteminfo = &g_pak->_M_iteminfos[i];

		if( !VFS_CHECK_FWRITE(fp,&iteminfo->_M_offset,sizeof(iteminfo->_M_offset)))
			goto LBL_FI_ERROR; 

		if( !VFS_CHECK_FWRITE(fp,&iteminfo->_M_size,sizeof(iteminfo->_M_offset)))
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

	return 1;

LBL_FI_ERROR:
	return 0;
}

int fwrite_header(FILE*fp)
{
	if( !fp )
		return 0;

	if( fseek(fp,0,SEEK_SET) != 0 )
		goto LBL_FH_ERROR;

	if( !VFS_CHECK_FWRITE(fp,&g_pak->_M_header._M_flag,sizeof(g_pak->_M_header._M_flag)))
		goto LBL_FH_ERROR;

	if( !VFS_CHECK_FWRITE(fp,&g_pak->_M_header._M_version,sizeof(g_pak->_M_header._M_version)))
		goto LBL_FH_ERROR;

	if( !VFS_CHECK_FWRITE(fp,&g_pak->_M_header._M_count,sizeof(g_pak->_M_header._M_count)))
		goto LBL_FH_ERROR;

	if( !VFS_CHECK_FWRITE(fp,&g_pak->_M_header._M_offset,sizeof(g_pak->_M_header._M_offset)))
		goto LBL_FH_ERROR;

	return 1;
LBL_FH_ERROR:
	return 0;
}


int pakfile_combine(FILE* fp_header,FILE*fp_iteminfo,FILE* fp_data,const char* outputfile)
{

	FILE *fp = NULL;
	FILE *fp_temp = NULL;

	int bufsize = 1024;
	int readsize =0;
	char buf[1024];

	if( !fp_header || !fp_iteminfo || !fp_data )
		return 0;

	fp = sfopen(outputfile,"wb+");
	if( !fp )
	{
		printf("error:pakfile_combine create outputfile %s failed\n",outputfile);
		return 0;
	}

	fseek(fp_header,0,SEEK_SET);
	fseek(fp_iteminfo,0,SEEK_SET);
	fseek(fp_data,0,SEEK_SET);

	printf("info:pakfile_combine begin pak header size=%d\n",file_getlen(fp_header));
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

	printf("info:pakfile_combine begin pak iteminfo size=%d\n",file_getlen(fp_iteminfo));
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

	printf("info:pakfile_combine begin pak data size=%d\n",file_getlen(fp_data));
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
	return 1;

LB_ERROR:
	VFS_SAFE_FCLOSE(fp);
	return 0;

}


int dir_pack( const char *path,const char* output ) 
{
	int i;

	pak_iteminfo* iteminfo = NULL;

	FILE* fp = NULL;

	FILE* fp_head = NULL;
	FILE* fp_iteminfo = NULL;
	FILE* fp_data = NULL;

	void* buf = NULL ;

	int	compress_buf_size = 0 ;
	void* compress_buf = NULL ;

	int compress_result = 0;
	int combinefile_result = 0;

	int tmp = 0;


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
		return 0;
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
		vfs_util_path_combine(filetemp,g_dir,iteminfo->_M_filename);

		fp = sfopen(filetemp,"rb");
		if( !fp )
		{
			printf("error:dir_pack open file %s failed\n",iteminfo->_M_filename);
			goto LBL_DP_ERROR;
		}

		iteminfo->_M_size = file_getlen(fp);
		iteminfo->_M_offset = ftell(fp_data);

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
			buf = malloc(iteminfo->_M_size);
			tmp = fread(buf,1,iteminfo->_M_size,fp);
			if( tmp != iteminfo->_M_size)
			{
				printf("error:dir_pack read file %s size=%d readsize=%d fpos=%d failed\n",
						iteminfo->_M_filename,
						iteminfo->_M_size,
						tmp,
						(int)ftell(fp));
				goto LBL_DP_ERROR;
			}

			VFS_SAFE_FCLOSE(fp);

			iteminfo->_M_crc32 = vfs_util_calc_crc32(buf,iteminfo->_M_size);

			compress_buf_size = vfs_util_compress_bound(VFS_COMPRESS_BZIP2,iteminfo->_M_size);
			compress_buf = malloc(compress_buf_size);

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
				iteminfo->_M_compress_crc32 = vfs_util_calc_crc32(compress_buf,compress_result);

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
		printf("\tsize=%d\n",iteminfo->_M_size);
		printf("\tcrc32=%d\n",iteminfo->_M_crc32);
		printf("\tcompress_type=%d\n",iteminfo->_M_compress_type);
		printf("\tcompress_size=%d\n",iteminfo->_M_compress_size);
		printf("\tcompress_crc32=%d\n",iteminfo->_M_compress_crc32);
		printf("\toffset=%d\n\n",iteminfo->_M_offset);

	}

	if( 0 == fwrite_iteminfos(fp_iteminfo) )
	{
		printf("error:dir_pack fwrite_iteminfos failed\n");
		goto LBL_DP_ERROR;
	}

	g_pak->_M_header._M_offset = ftell(fp_iteminfo)+sizeof(pak_header);
	if( 0 == fwrite_header(fp_head) )
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

	return 0;
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
	strcpy(g_dir,path);
	g_dirlen = strlen(g_dir);

	if( 0 == pak_begin(path) )
	{
		printf("error:pak_begin failed\n ");
		goto LB_ERROR;
	}
	
	if( 0 == dir_collect_fileinfo(path))
	{
		printf("error:dir_collect_fileinfo failed \n");
		goto LB_ERROR;
	}


	if( 0 == dir_pack(path,outfile) )
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
