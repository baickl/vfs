#include "vfs/vfs.h"
#include "vfs/common.h"
#include <stdio.h>
#include <string.h>

static vfs * g_vfs = NULL;

int vfs_pak_sort_cmp(const void*a,const void*b)
{

	pak* _a;
	pak* _b;
	
	_a = *(pak**)a;
	_b = *(pak**)b;

	return stricmp(_a->_M_filename,_b->_M_filename);
}

int vfs_pak_search_cmp(const void*key,const void*item)
{

	char*_key;
	pak* _item;

	_key  = (char*)key;
	_item = *(pak**)item;
	return stricmp(_key,_item->_M_filename);
}

void vfs_pak_sort()
{
	qsort((void*)g_vfs->_M_paks,g_vfs->_M_count,sizeof(pak*),vfs_pak_sort_cmp);
}

int vfs_pak_search(const char* pakfile)
{
	int ret = -1;
	pak** p=NULL;

	if( !g_vfs || !pakfile)
		return -1;

	p = (pak**)bsearch(pakfile,g_vfs->_M_paks,g_vfs->_M_count,sizeof(pak*),vfs_pak_search_cmp);
	if( !p )
		return -1;

	return (p - g_vfs->_M_paks);
}

VFS_BOOL vfs_create()
{
	if( g_vfs )
		return VFS_TRUE;

	g_vfs = (vfs*)malloc(sizeof(vfs));
	if( !g_vfs )
		return VFS_FALSE;

	g_vfs->_M_count = 0;
	g_vfs->_M_maxcount = 0;
	g_vfs->_M_paks = NULL;

	return VFS_TRUE;
}

void vfs_destroy()
{
	int i;

	if( !g_vfs )
		return ;

	for( i = 0; i<g_vfs->_M_count; ++i )
	{
		pak_close(g_vfs->_M_paks[i]);
	}

	VFS_SAFE_FREE(g_vfs->_M_paks);
	VFS_SAFE_FREE(g_vfs);
}

VFS_BOOL vfs_pak_add(const char* pakfile )
{
	pak* p;
	pak** _paks;

	if(vfs_pak_search(pakfile) >= 0 )
		return VFS_TRUE;

	p = pak_open(pakfile);
	if( !p )
		return VFS_FALSE;

	if( g_vfs->_M_count >= g_vfs->_M_maxcount )
	{
		if( g_vfs->_M_count == 0 )
		{
			g_vfs->_M_maxcount = 32;
			g_vfs->_M_paks = (pak**)malloc(g_vfs->_M_maxcount*sizeof(pak*));
			if( !g_vfs->_M_paks )
			{
				g_vfs->_M_maxcount = 0;
				pak_close(p);
				return VFS_FALSE;
			}
		}
		else
		{
			g_vfs->_M_maxcount += 32;
			_paks = (pak**)realloc(g_vfs->_M_paks,g_vfs->_M_maxcount*sizeof(pak*));
			if( !_paks )
			{
				g_vfs->_M_maxcount -= 32;
				pak_close(p);
				return VFS_FALSE;
			}

			/*
			 * 拿到新地址
			 */
			g_vfs->_M_paks = _paks;
		}
	}
	
	/* 新增 */
	g_vfs->_M_paks[g_vfs->_M_count++] = p;
	vfs_pak_sort();

	return VFS_TRUE;
}

VFS_BOOL vfs_pak_del(const char* pakfile )
{

	int index;

	if( !g_vfs )
		return VFS_FALSE;

	if( !pakfile )
		return VFS_FALSE;

	index = vfs_pak_search(pakfile);
	if( index < 0 || index >= g_vfs->_M_count )
		return VFS_FALSE;

	pak_close(g_vfs->_M_paks[index]);\
	g_vfs->_M_paks[index]= g_vfs->_M_paks[g_vfs->_M_count -1];
	--g_vfs->_M_count;

	vfs_pak_sort();
	return VFS_TRUE;
}


int vfs_pak_get_count()
{
	if( g_vfs )
		return g_vfs->_M_count;
	return 0;
}

pak* vfs_pak_get_index(int idx )
{
	if( g_vfs && idx >= 0 && idx <= g_vfs->_M_count )
		return g_vfs->_M_paks[idx];

	return NULL;
}

pak* vfs_pak_get_name(const char* pakfile)
{	
	pak* p;
	int index;

	index = vfs_pak_search(pakfile);
	if( index < 0 || index >= g_vfs->_M_count )
		return NULL;

	p = g_vfs->_M_paks[index];
	return p;
}

vfs_file* vfs_fcreate(void*buf,size_t size)
{
	vfs_file* vff;

	vff = (vfs_file*)malloc(sizeof(vfs_file));
	if( !vff )
	{
		VFS_SAFE_FREE(buf);
		return NULL;
	}

	if( !buf || !size)
	{
		vff->_M_buffer = 0;
		vff->_M_size = 0;
		vff->_M_position = 0;
	}
	else
	{
		vff->_M_buffer = (void*)malloc(size);
		if( !vff->_M_buffer )
		{
			VFS_SAFE_FREE(buf);
			return NULL;
		}
		vff->_M_position = 0;
		vff->_M_size = size;

		memcpy(vff->_M_buffer,buf,size);
	}
	
	return vff;
}

vfs_file* vfs_fopen(const char* file,const char* mode )
{
	int i,index ;
	int size;
	void* buf;
	pak_iteminfo* iteminfo;
	vfs_file* vff;

	if( !g_vfs || !file )
		return NULL;

	for( i = 0; i<g_vfs->_M_count; ++i )
	{
		index = pak_item_locate(g_vfs->_M_paks[i],file);
		if(index < 0 )
			continue;

		iteminfo = pak_item_getinfo(g_vfs->_M_paks[i],index);
		if( !iteminfo )
			continue;

		size = iteminfo->_M_size;
		buf = (void*)malloc(size);
		if( !buf )
			return NULL;
		
		if( VFS_TRUE != pak_item_unpack_index(g_vfs->_M_paks[i],index,buf,size) ) 
		{
			VFS_SAFE_FREE(buf);
			return NULL;
		}
		
		vff = vfs_fcreate(0,0);
		if( !vff )
		{
			VFS_SAFE_FREE(buf);
			return NULL;
		}
		vff->_M_buffer = buf;
		vff->_M_size = size;
		vff->_M_position = 0;
		
		return vff;
	}

	return NULL;
}

void vfs_fclose(vfs_file* file)
{
	if( file )
	{
		VFS_SAFE_FREE(file->_M_buffer);
		VFS_SAFE_FREE(file);
	}
}


VFS_BOOL vfs_feof(vfs_file* file )
{
	if( file && file->_M_position == file->_M_size -1 )
		return VFS_TRUE;
	else
		return VFS_FALSE;
}

int vfs_ftell(vfs_file* file)
{
	if( file )
		return file->_M_position;

	return -1;
}

int vfs_fsize( vfs_file* file )
{
	if( !file )
		return 0;

	return file->_M_size;
}

int vfs_fseek(vfs_file* file,int pos, int mod )
{
	int _pos;
	if( !file )
		return -1;

	if( mod == SEEK_CUR )
	{
		_pos = file->_M_position + pos;
		if( _pos >= 0 && _pos < file->_M_size  )
			file->_M_position = pos;
	}
	else if( mod == SEEK_END )
	{
		_pos = file->_M_size -1 + pos;
		if( _pos >= 0 && _pos < file->_M_size )
			file->_M_position = _pos;
	}
	else
	{
		if( pos >= 0 && pos < file->_M_size  )
			file->_M_position = pos;
	}

	return vfs_ftell(file);
}

size_t vfs_fread( void* buf , size_t size , size_t count , vfs_file*fp )
{

	size_t i;
	int realread;
	char* p;

	if(!fp)
		return 0;

	if(!buf)
		return 0;
	
	if(!size || !count )
		return 0;
	
	p = (char*)buf;
	realread = 0;

	for( i = 0; i<count; ++i )
	{
		if( vfs_feof(fp) )
			break;

		if( (fp->_M_size - fp->_M_position - 1 ) >= (int)size )
		{
			memcpy(p,&((char*)fp->_M_buffer)[fp->_M_position],size);
			fp->_M_position += size;
			p += size;
			++realread;
		}
	}

	return realread;
}

size_t vfs_fwrite(void* buf , size_t size , size_t count , vfs_file*fp )
{
	size_t i;
	int realwrite;
	char* p;
	
	void* tmp;
	
	if( !fp )
		return 0;

	if( !buf )
		return 0;

	if( !size || !count )
		return 0;

	p = (char*)buf;
	for( i = 0; i<count; ++i )
	{
		if( (fp->_M_size - fp->_M_position - 1 ) < (int)size )
		{
			tmp = (void*)realloc(fp->_M_buffer,fp->_M_size + size );
			if( !tmp )
				break;

			fp->_M_buffer = tmp;
			fp->_M_size += size;
		}

		memcpy(&((char*)fp->_M_buffer)[fp->_M_position],p,size);
		fp->_M_position += size;
		p += size;
		++realwrite;

	}

	return realwrite;
}


VFS_BOOL vfs_fflush(vfs_file* file,const char* saveas)
{

	FILE* fp;
	int offset;

	size_t realsize ;
	char buf[512+1];

	if( !file || !saveas  )
		return VFS_FALSE;
	
	fp = fopen(saveas,"wb+");
	if( !fp )
		return VFS_FALSE;

	offset = vfs_ftell(file);
	while( !vfs_feof(file) )
	{
		realsize = vfs_fread(buf,1,buf,file);
		if( realsize > 0 )
		{
			buf[realsize] = 0;
			if( fwrite(buf,1,realsize,fp) != realsize )
			{
				VFS_SAFE_FCLOSE(fp);
				vfs_fseek(file,offset,SEEK_SET);
				remove(saveas);
				return VFS_FALSE;
			}
		}

		VFS_SAFE_FCLOSE(fp);
		vfs_fseek(file,offset,SEEK_SET);
	}

	VFS_SAFE_FCLOSE(fp);
	return VFS_TRUE;
}
