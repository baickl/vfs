#include "pak/pak.h"
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <dirent.h>

#define SAFE_FCLOSE(x)	if(x){fclose(x);x=NULL;}
#define SAFE_FREE(x)	if(x){free((void*)x);	x=NULL;}

#define CHECK_FWRITE(f,b,l) fwrite((const void*)b,1,l,f) !=l?0:1 

static pak *g_pak = NULL;
static int  g_maxcount = 0;
static char g_dir[PAK_MAX_FILENAME+1];

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

	g_pak = _pak;
	g_pak->_M_header._M_flag = 'pakx';
	g_pak->_M_header._M_version = PAK_VERSION;
	g_pak->_M_header._M_count = 0;

	g_maxcount = 0;
	memset(g_dir,0,sizeof(g_dir));

	return 1;
}

int pak_additeminfo( const char* path )
{
	pak_iteminfo* iteminfo;

	if( g_pak->_M_header._M_count >= g_maxcount )
	{
		if( g_pak->_M_header._M_count == 0 )
			g_maxcount = 256;
		else
			g_maxcount += g_maxcount;

		g_pak->_M_iteminfos = (pak_iteminfo*)realloc(g_pak->_M_iteminfos,g_maxcount*sizeof(pak_iteminfo));
		if( !g_pak->_M_iteminfos )
			return 0;
	}

	iteminfo = &g_pak->_M_iteminfos[g_pak->_M_header._M_count++];

	memset(iteminfo,0,sizeof(pak_iteminfo));
	strcpy(iteminfo->_M_filename,path);


	printf("add file:%s\n",iteminfo->_M_filename);
	return 1;

}

int dir_collect_fileinfo( const char *path )
{
	DIR* dir;
	struct dirent *entry;

	char find_full[PAK_MAX_FILENAME+1];
	char path_temp[PAK_MAX_FILENAME+1];

	strcpy(find_full,path);
	
	dir = opendir(path);
	if( NULL == dir )
		return 0;

	while( (entry=readdir(dir)) != NULL )
	{

		if( entry->d_type & DT_DIR )
		{
			if( strcmp(entry->d_name,".")  == 0 ||
				strcmp(entry->d_name,"..") == 0 )
				continue;

			memset(path_temp,0,sizeof(path_temp));
			strcpy(path_temp,path);
			strcat(path_temp,"/");
			strcat(path_temp,entry->d_name);

			if( 0 == dir_collect_fileinfo(path_temp) )
				goto ERROR;
		}		
		else
		{
			memset(path_temp,0,sizeof(path_temp));
			strcpy(path_temp,path);
			strcat(path_temp,"/");
			strcat(path_temp,entry->d_name);
			
			if( 0 == pak_additeminfo(path_temp) )
				goto ERROR;
		}
	}

	closedir(dir);
	return 1;

ERROR:
	closedir(dir);
	return 0;

}
/*  
int dir_collect_fileinfo( const char *path )
{
	long handle;
	struct _finddata_t fd;

	char find_full[PAK_MAX_FILENAME+1];
	char path_temp[PAK_MAX_FILENAME+1];

	strcpy(find_full,path);
	strcat(find_full,"*");
	
	handle = _findfirst(find_full,&fd);
	if( -1 == handle )
		return 0;

	while( !_findnext(handle,&fd) )
	{

		if( fd.attrib & A_SUBDIR )
		{
			if( strcmp(fd.name,".")  == 0 ||
				strcmp(fd.name,"..") == 0 )
				continue;

			memset(path_temp,0,sizeof(path_temp));
			strcpy(path_temp,path);
			strcat(path_temp,fd.name);

			if( 0 == dir_collect_fileinfo(path_temp) )
				goto ERROR;
		}		
		else
		{
			memset(path_temp,0,sizeof(path_temp));
			strcpy(path_temp,path);
			strcat(path_temp,fd.name);
			
			if( 0 == pak_additeminf(path_temp) )
				goto ERROR;
		}
	}

	_findclose(handle);
	return 1;

ERROR:
	_findclose(handle);
	return 0;
}
*/

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
	pak_iteminfo * iteminfo;

	if( !fp )
		return 0;

	if( fseek(fp,0,SEEK_SET) != 0 )
		return 0;

	if( g_pak->_M_header._M_count <= 0 )
		return 0;

	qsort(g_pak->_M_iteminfos,g_pak->_M_header._M_count,sizeof(pak_iteminfo),pak_item_sort_cmp);

	for( i = 0; i<g_pak->_M_header._M_count; ++i )
	{
		iteminfo = &g_pak->_M_iteminfos[i];

		if( !CHECK_FWRITE(fp,&iteminfo->_M_offset,sizeof(iteminfo->_M_offset)))
			goto LBL_FI_ERROR; 

		if( !CHECK_FWRITE(fp,&iteminfo->_M_size,sizeof(iteminfo->_M_offset)))
			goto LBL_FI_ERROR; 

		if( !CHECK_FWRITE(fp,&iteminfo->_M_crc32,sizeof(iteminfo->_M_crc32)))
			goto LBL_FI_ERROR; 

		if( !CHECK_FWRITE(fp,&iteminfo->_M_compress_type,sizeof(iteminfo->_M_compress_type)))
			goto LBL_FI_ERROR; 

		if( !CHECK_FWRITE(fp,&iteminfo->_M_compress_size,sizeof(iteminfo->_M_compress_size)))
			goto LBL_FI_ERROR; 

		if( !CHECK_FWRITE(fp,&iteminfo->_M_compress_crc32,sizeof(iteminfo->_M_compress_crc32)))
			goto LBL_FI_ERROR; 

		int len = strlen(iteminfo->_M_filename);
		if( !CHECK_FWRITE(fp,&len,sizeof(len)))
			goto LBL_FI_ERROR; 

		if( !CHECK_FWRITE(fp,iteminfo->_M_filename,len))
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
		return 0;

	if( !CHECK_FWRITE(fp,&g_pak->_M_header._M_flag,sizeof(g_pak->_M_header._M_flag)))
		goto LBL_FH_ERROR;

	if( !CHECK_FWRITE(fp,&g_pak->_M_header._M_version,sizeof(g_pak->_M_header._M_version)))
		goto LBL_FH_ERROR;

	if( !CHECK_FWRITE(fp,&g_pak->_M_header._M_count,sizeof(g_pak->_M_header._M_count)))
		goto LBL_FH_ERROR;

	if( !CHECK_FWRITE(fp,&g_pak->_M_header._M_offset,sizeof(g_pak->_M_header._M_offset)))
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

	fp = fopen(outputfile,"wb+");
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
			if( !CHECK_FWRITE(fp,buf,readsize))
				goto ERROR;
		}
	}

	printf("info:pakfile_combine begin pak iteminfo size=%d\n",file_getlen(fp_iteminfo));
	fp_temp = fp_iteminfo;
	while(!feof(fp_temp))
	{
		readsize = fread(buf,1,bufsize,fp_temp);
		if(readsize>0)
		{
			if( !CHECK_FWRITE(fp,buf,readsize))
				goto ERROR;
		}
	}

	printf("info:pakfile_combine begin pak data size=%d\n",file_getlen(fp_data));
	fp_temp = fp_data;
	while(!feof(fp_temp))
	{
		readsize = fread(buf,1,bufsize,fp_temp);
		if(readsize>0)
		{
			if( !CHECK_FWRITE(fp,buf,readsize))
				goto ERROR;
		}
	}
	
	SAFE_FCLOSE(fp);
	return 1;

ERROR:
	SAFE_FCLOSE(fp);
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
	
	remove("pak_header.tmp");
	remove("pak_iteminfo.tmp");
	remove("pak_data.tmp");

	fp_head = fopen("pak_header.tmp","wb+");
	if( !fp_head )
	{
		printf("error:dir_pack create pak_header.tmp failed\n");
		return 0;
	}

	fp_iteminfo = fopen("pak_iteminfo.tmp","wb+");
	if( !fp_iteminfo )
	{
		printf("error:dir_pack create pak_iteminfo.tmp failed\n");
		goto LBL_DP_ERROR;
	}

	fp_data = fopen("pak_data.tmp","wb+");
	if( !fp_data )
	{
		printf("error:dir_pack create pak_data.tmp failed\n");
		goto LBL_DP_ERROR;
	}

	for( i = 0; i<g_pak->_M_header._M_count; ++i )
	{
		iteminfo = &g_pak->_M_iteminfos[i];

		fp = fopen(iteminfo->_M_filename,"r");
		if( !fp )
		{
			printf("error:dir_pack open file %s failed\n",iteminfo->_M_filename);
			goto LBL_DP_ERROR;
		}

		iteminfo->_M_size = file_getlen(fp);
		iteminfo->_M_offset = ftell(fp_data);

		if( iteminfo->_M_size <= 0 )
		{
			SAFE_FCLOSE(fp);
			iteminfo->_M_size = 0;
			iteminfo->_M_crc32 = 0;

			iteminfo->_M_compress_type = PAK_COMPRESS_NONE;
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
				printf("error:dir_pack read file %s size=%d readsize=%d fpos=%d failed\n",iteminfo->_M_filename,iteminfo->_M_size,tmp,ftell(fp));
				goto LBL_DP_ERROR;
			}

			SAFE_FCLOSE(fp);

			iteminfo->_M_crc32 = pak_util_calc_crc32(buf,iteminfo->_M_size);

			compress_buf_size = pak_util_compress_bound(PAK_COMPRESS_BZIP2,iteminfo->_M_size);
			compress_buf = malloc(compress_buf_size);

			compress_result = pak_util_compress(PAK_COMPRESS_BZIP2,buf,iteminfo->_M_size,compress_buf,compress_buf_size);
			if(compress_result == 0 || compress_result >= iteminfo->_M_size)
			{
				iteminfo->_M_compress_type = PAK_COMPRESS_NONE;
				iteminfo->_M_compress_size = 0;
				iteminfo->_M_compress_crc32 = 0;

				SAFE_FREE(compress_buf);
				compress_buf_size = 0;

				if( !CHECK_FWRITE(fp_data,buf,iteminfo->_M_size))
				{
					printf("error:dir_pack pack file[no compress] %s failed\n",iteminfo->_M_filename);
					goto LBL_DP_ERROR;
				}

				SAFE_FREE(buf);
			}
			else
			{
				iteminfo->_M_compress_type = PAK_COMPRESS_BZIP2;
				iteminfo->_M_compress_size = compress_result;
				iteminfo->_M_compress_crc32 = pak_util_calc_crc32(compress_buf,compress_result);

				SAFE_FREE(buf);

				if( !CHECK_FWRITE(fp_data,compress_buf,compress_result))
				{
					printf("error:dir_pack pack file[compress] %s failed\n",iteminfo->_M_filename);
					goto LBL_DP_ERROR;
				}

				SAFE_FREE(compress_buf);
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

	SAFE_FCLOSE(fp_data);
	SAFE_FCLOSE(fp_iteminfo);
	SAFE_FCLOSE(fp_head);

	return combinefile_result;

LBL_DP_ERROR:
	SAFE_FREE(buf);
	SAFE_FREE(compress_buf);
	SAFE_FCLOSE(fp);
	SAFE_FCLOSE(fp_data);
	SAFE_FCLOSE(fp_iteminfo);
	SAFE_FCLOSE(fp_head);

	return 0;
}

void pak_end( const char *path )
{
	if( g_pak )
	{
		SAFE_FREE(g_pak->_M_iteminfos);
		SAFE_FREE(g_pak);
	}
	
	remove("pak_header.tmp");
	remove("pak_iteminfo.tmp");
	remove("pak_data.tmp");
}

int main( int argc,char *argv[] )
{
	if(argc != 3 )
	{
		printf("usage: pack_dir <directory> <outputfile>\n");
		return -1;
	}

	const char* path = argv[1];
	const char* outfile = argv[2];

	printf("pack_dir %s %s\n",path,outfile);

	memset(g_dir,0,sizeof(g_dir));
	strcpy(g_dir,path);

	if( 0 == pak_begin(path) )
	{
		printf("error:pak_begin failed\n ");
		goto ERROR;
	}
	
	if( 0 == dir_collect_fileinfo(path))
	{
		printf("error:dir_collect_fileinfo failed \n");
		goto ERROR;
	}

	if( 0 == dir_pack(path,outfile) )
	{
		printf("error:dir_pack failed\n");
		goto ERROR;
	}
	
	pak_end(path);
	return 0;

ERROR:
	pak_end(path);
	return -1;
}