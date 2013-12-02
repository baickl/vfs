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
#include "vfs_private.h"
#include "pak.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

vfs *g_vfs = NULL;

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


var32 vfs_pak_sort_cmp(const void*a,const void*b)
{

	pak* _a;
	pak* _b;
	
	_a = *(pak**)a;
	_b = *(pak**)b;

	return stricmp(_a->_M_filename,_b->_M_filename);
}

var32 vfs_pak_search_cmp(const void*key,const void*item)
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

var32 vfs_pak_search(const char* pakfile)
{
	var32 ret = -1;
	pak** p=NULL;

	if( !g_vfs || !pakfile)
		return -1;

	p = (pak**)bsearch(pakfile,g_vfs->_M_paks,g_vfs->_M_count,sizeof(pak*),vfs_pak_search_cmp);
	if( !p )
		return -1;

	return (p - g_vfs->_M_paks);
}

VFS_BOOL vfs_create(const char* workpath,struct vfs_memmgr*mm)
{
	if( g_vfs )
		return VFS_TRUE;

    if( !workpath )
        return VFS_FALSE;

	g_vfs = (vfs*)(mm?mm->malloc(sizeof(vfs)):malloc(sizeof(vfs)));
	if( !g_vfs )
		return VFS_FALSE;

	g_vfs->_M_count = 0;
	g_vfs->_M_maxcount = 0;
	g_vfs->_M_paks = NULL;
    strcpy(g_vfs->_M_workpath,workpath);

    if( mm )
    {
        g_vfs->malloc = mm->malloc;
        g_vfs->realloc = mm->realloc;
        g_vfs->free = mm->free;
    }
    else
    {
        g_vfs->malloc = &malloc;
        g_vfs->realloc = &realloc;
        g_vfs->free = &free;
    }

	return VFS_TRUE;
}

void vfs_destroy()
{
	var32 i;

	if( !g_vfs )
		return ;

	for( i = 0; i<g_vfs->_M_count; ++i )
	{
		pak_close(g_vfs->_M_paks[i]);
	}

	if(g_vfs->_M_paks){
        vfs_free(g_vfs->_M_paks);
        g_vfs->_M_paks;
    }
	
    vfs_free(g_vfs);
    g_vfs=NULL;
}

VFS_BOOL vfs_add_pak( const char* pakfile )
{
	pak* p;
	pak** _paks;

    char* prefix = NULL;
    char _filepath[VFS_MAX_FILENAME+1];
    char _fullpath[VFS_MAX_FILENAME+1];

    if( !g_vfs || !pakfile )
        return VFS_FALSE;

    if( !vfs_util_path_clone(_filepath,pakfile) )
        return VFS_FALSE;

    if( (prefix = strstr(_filepath,g_vfs->_M_workpath)) == NULL )
    {
        prefix = _filepath;
    }

    vfs_util_path_remove_filename(prefix);
    vfs_util_path_combine(_fullpath,g_vfs->_M_workpath,pakfile);

	if(vfs_pak_search(_fullpath) >= 0 )
		return VFS_TRUE;

	p = pak_open(_fullpath,prefix);
	if( !p )
		return VFS_FALSE;

	if( g_vfs->_M_count >= g_vfs->_M_maxcount )
	{
		if( g_vfs->_M_count == 0 )
		{
			g_vfs->_M_maxcount = 16;
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
			g_vfs->_M_maxcount += 16;
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

VFS_BOOL vfs_remove_pak(const char* pakfile )
{

	var32 index;

	if( !g_vfs )
		return VFS_FALSE;

	if( !pakfile )
		return VFS_FALSE;

	index = vfs_pak_search(pakfile);
	if( index < 0 || index >= g_vfs->_M_count )
		return VFS_FALSE;

	pak_close(g_vfs->_M_paks[index]);
	g_vfs->_M_paks[index]= g_vfs->_M_paks[g_vfs->_M_count -1];
	--g_vfs->_M_count;

	vfs_pak_sort();
	return VFS_TRUE;
}

void* vfs_malloc(size_t size )
{
    if( g_vfs )
        return g_vfs->malloc(size);
    return NULL;
}

void* vfs_realloc(void*p,size_t size)
{
    if( g_vfs )
        return g_vfs->realloc(p,size);
    return NULL;
}

void vfs_free(void*p)
{
    if( g_vfs && p )
        g_vfs->free(p);
}


var32 vfs_get_pak_count()
{
	if( g_vfs )
		return g_vfs->_M_count;
	return 0;
}

pak* vfs_get_pak_index(var32 idx )
{
	if( g_vfs && idx >= 0 && idx <= g_vfs->_M_count )
		return g_vfs->_M_paks[idx];

	return NULL;
}

pak* vfs_get_pak_name(const char* pakfile)
{	
	pak* p;
	var32 index;

	index = vfs_pak_search(pakfile);
	if( index < 0 || index >= g_vfs->_M_count )
		return NULL;

	p = g_vfs->_M_paks[index];
	return p;
}
