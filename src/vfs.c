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
#include "pak_plugin.h"
#include <stdio.h>
#include <string.h>

vfs *g_vfs = NULL;

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


/************************************************************************/
/* pak search and locate                                                */
/************************************************************************/
static var32 vfs_archive_obj_sort_cmp(const void*a,const void*b)
{
	vfs_archive_obj* _a;
	vfs_archive_obj* _b;
	
	_a = *(vfs_archive_obj**)a;
	_b = *(vfs_archive_obj**)b;

	return stricmp(_a->plugin->plugin_archive_get_name(_a->archive),
                   _b->plugin->plugin_archive_get_name(_b->archive));
}

static var32 vfs_archive_obj_search_cmp(const void*key,const void*item)
{
	char*_key;
	vfs_archive_obj* _item;

	_key  = (char*)key;
	_item = *(vfs_archive_obj**)item;
	return stricmp(_key,_item->plugin->plugin_archive_get_name(_item->archive));
}

static void vfs_archive_obj_sort()
{
	qsort((void*)g_vfs->_M_archives,g_vfs->_M_count,sizeof(vfs_archive_obj*),vfs_archive_obj_sort_cmp);
}

static var32 vfs_archive_obj_search(const char* archive)
{
	var32 ret = -1;
	vfs_archive_obj** p=NULL;

	if( !g_vfs || !archive)
		return -1;

	p = (vfs_archive_obj**)bsearch(archive,g_vfs->_M_archives,g_vfs->_M_count,sizeof(vfs_archive_obj*),vfs_archive_obj_search_cmp);
	if( !p )
		return -1;

	return (p - g_vfs->_M_archives);
}


/************************************************************************/
/* plugin search and locate                                             */
/************************************************************************/
static var32 vfs_plugin_sort_cmp(const void*a,const void*b)
{
    vfs_plugin* _a;
    vfs_plugin* _b;

    _a = *(vfs_plugin**)a;
    _b = *(vfs_plugin**)b;

    return stricmp(_a->plugin_archive_get_plugin_name(),_b->plugin_archive_get_plugin_name());
}

static var32 vfs_plugin_search_cmp(const void*key,const void*item)
{
    char*_key;
    vfs_plugin* _item;

    _key  = (char*)key;
    _item = *(vfs_plugin**)item;
    return stricmp(_key,_item->plugin_archive_get_plugin_name());
}

static void vfs_plugin_sort()
{
    qsort((void*)g_vfs->_M_plugins,g_vfs->_M_plugins_count,sizeof(vfs_plugin*),vfs_plugin_sort_cmp);
}

static var32 vfs_plugin_search(const char* pluginname)
{
    var32 ret = -1;
    vfs_plugin** p=NULL;

    if( !g_vfs || !pluginname)
        return -1;

    p = (vfs_plugin**)bsearch(pluginname,g_vfs->_M_plugins,g_vfs->_M_plugins_count,sizeof(vfs_plugin*),vfs_plugin_search_cmp);
    if( !p )
        return -1;

    return (p - g_vfs->_M_plugins);
}

static vfs_plugin* vfs_plugin_check_archive_type(const char* archive)
{
    int i;
    vfs_plugin* plugin;

    if( !g_vfs )
        return NULL;

    for( i = 0; i<g_vfs->_M_plugins_count; ++i )
    {
        plugin = g_vfs->_M_plugins[i];
        if( VFS_TRUE == plugin->plugin_archive_check_type(archive) )
            return plugin;
    }

    return NULL;
}

/************************************************************************/
/* vfs implement                                                        */
/************************************************************************/

VFS_BOOL vfs_create(const char* sdk_version,const char* workpath)
{
	if( g_vfs )
		return VFS_TRUE;

    if( !workpath || !sdk_version )
        return VFS_FALSE;

    if( stricmp(VFS_SDK_VERSION,sdk_version) != 0 )
        return VFS_FALSE;

	g_vfs = (vfs*)(malloc(sizeof(vfs)));
	if( !g_vfs )return VFS_FALSE;

	g_vfs->_M_count = 0;
	g_vfs->_M_maxcount = 0;
	g_vfs->_M_archives = NULL;

    g_vfs->_M_plugins_count = 0;
    g_vfs->_M_plugins_maxcount = 0;
    g_vfs->_M_plugins = NULL;

    strcpy(g_vfs->_M_workpath,workpath);

    /* 注册PAK组件 */
    if( VFS_TRUE != vfs_register_archive_plugin(g_plugin_pak.plugin_archive_get_plugin_name(),g_plugin_pak) )
    {
        vfs_destroy();
        return VFS_FALSE;
    }

	return VFS_TRUE;
}

void vfs_destroy()
{
	var32 i;

    vfs_archive_obj* archive;

	if( !g_vfs )return ;

	for( i = 0; i<g_vfs->_M_count; ++i )
	{
		archive = g_vfs->_M_archives[i];
        archive->plugin->plugin_archive_close(archive->archive);
	}

	if(g_vfs->_M_archives){
        free(g_vfs->_M_archives);
        g_vfs->_M_archives = NULL;
    }

    if(g_vfs->_M_plugins){
        free(g_vfs->_M_plugins);
        g_vfs->_M_plugins = NULL;
    }
	
    free(g_vfs);
    g_vfs = NULL;
}


VFS_BOOL vfs_register_archive_plugin(const char*pluginname,vfs_plugin plugin)
{
    vfs_plugin* _plugin;
    vfs_plugin** _plugins;

    if(vfs_plugin_search(pluginname) >= 0 )
        return VFS_TRUE;

    if( g_vfs->_M_plugins_count >= g_vfs->_M_plugins_maxcount )
	{
		if( g_vfs->_M_plugins_count == 0 )
		{
			g_vfs->_M_plugins_maxcount = 16;
			g_vfs->_M_plugins = (vfs_plugin**)malloc(g_vfs->_M_plugins_maxcount*sizeof(vfs_plugin*));
			if( !g_vfs->_M_plugins )
			{
				g_vfs->_M_plugins_maxcount = 0;
				return VFS_FALSE;
			}
		}
		else
		{
			g_vfs->_M_plugins_maxcount += 16;
			_plugins = (vfs_plugin**)realloc(g_vfs->_M_plugins,g_vfs->_M_plugins_maxcount*sizeof(vfs_plugin*));
			if( !_plugins )
			{
				g_vfs->_M_plugins_maxcount -= 16;
				return VFS_FALSE;
			}

			/*
			 * 拿到新地址
			 */
			g_vfs->_M_plugins = _plugins;
		}
	}

    _plugin = (vfs_plugin*)malloc(sizeof(vfs_plugin));
    if( !_plugin )return VFS_FALSE;
    memcpy(_plugin,&plugin,sizeof(vfs_plugin));
    g_vfs->_M_plugins[g_vfs->_M_plugins_count++] = _plugin;

    return VFS_TRUE;
}

void vfs_unregister_archive_plugin(const char*pluginname )
{
    var32 index;
    if( !g_vfs )
        return;

    if( !pluginname )
        return;

    index = vfs_plugin_search(pluginname);
    if( index < 0 || index >= g_vfs->_M_plugins_count )
        return;

    free(g_vfs->_M_plugins[index]);
    g_vfs->_M_plugins[index]= g_vfs->_M_plugins[g_vfs->_M_plugins_count -1];
    --g_vfs->_M_plugins;

    vfs_plugin_sort();
}

VFS_BOOL vfs_add_archive( const char* archive,const char* passwd )
{
	vfs_archive_obj*  p;
	vfs_archive_obj** _archives;

    vfs_plugin* plugin;

    char* prefix = NULL;
    char _filepath[VFS_MAX_FILENAME+1];
    char _fullpath[VFS_MAX_FILENAME+1];

    if( !g_vfs || !archive )
        return VFS_FALSE;

    if( !vfs_util_path_clone(_filepath,archive) )
        return VFS_FALSE;

    if( (prefix = strstr(_filepath,g_vfs->_M_workpath)) == NULL )
        prefix = _filepath;

    vfs_util_path_remove_filename(prefix);
    vfs_util_path_combine(_fullpath,g_vfs->_M_workpath,archive);

	if(vfs_archive_obj_search(_fullpath) >= 0 )
		return VFS_TRUE;

    plugin = vfs_plugin_check_archive_type(_fullpath);
    if( !plugin )
        return VFS_FALSE;

    p = (vfs_archive_obj*)malloc(sizeof(vfs_archive_obj));
    if( !p )
        return VFS_FALSE;

    p->plugin = plugin;
	p->archive = p->plugin->plugin_archive_open(_fullpath,prefix,passwd);
	if( !p->archive)
    {
        free(p);
        return VFS_FALSE;
    }

	if( g_vfs->_M_count >= g_vfs->_M_maxcount )
	{
		if( g_vfs->_M_count == 0 )
		{
			g_vfs->_M_maxcount = 16;
			g_vfs->_M_archives = (vfs_archive_obj**)malloc(g_vfs->_M_maxcount*sizeof(vfs_archive_obj*));
			if( !g_vfs->_M_archives )
			{
				g_vfs->_M_maxcount = 0;
                p->plugin->plugin_archive_close(p->archive);
                free(p);

				return VFS_FALSE;
			}
		}
		else
		{
			g_vfs->_M_maxcount += 16;
			_archives = (vfs_archive_obj**)realloc(g_vfs->_M_archives,g_vfs->_M_maxcount*sizeof(vfs_archive_obj*));
			if( !_archives )
			{
				g_vfs->_M_maxcount -= 16;
				
                p->plugin->plugin_archive_close(p->archive);
                free(p);

				return VFS_FALSE;
			}

			/*
			 * 拿到新地址
			 */
			g_vfs->_M_archives = _archives;
		}
	}
	
	/* 新增 */
	g_vfs->_M_archives[g_vfs->_M_count++] = p;
	vfs_archive_obj_sort();

	return VFS_TRUE;
}

VFS_BOOL vfs_remove_archive(const char* archive )
{

	var32 index;
    vfs_archive_obj* p;

	if( !g_vfs )
		return VFS_FALSE;

	if( !archive )
		return VFS_FALSE;

	index = vfs_archive_obj_search(archive);
	if( index < 0 || index >= g_vfs->_M_count )
		return VFS_FALSE;

    p = g_vfs->_M_archives[index];
    p->plugin->plugin_archive_close(p->archive);
	free(p);

	g_vfs->_M_archives[index]= g_vfs->_M_archives[g_vfs->_M_count -1];
	--g_vfs->_M_count;

	vfs_archive_obj_sort();
	return VFS_TRUE;
}


var32 vfs_get_archive_count()
{
	if( g_vfs )
		return g_vfs->_M_count;
	return 0;
}

vfs_archive_obj* vfs_get_archive_index(var32 idx )
{
	if( g_vfs && idx >= 0 && idx <= g_vfs->_M_count )
		return g_vfs->_M_archives[idx];

	return NULL;
}

vfs_archive_obj* vfs_get_archive_name(const char* archive)
{	
	vfs_archive_obj* p;
	var32 index;

	index = vfs_archive_obj_search(archive);
	if( index < 0 || index >= g_vfs->_M_count )
		return NULL;

	p = g_vfs->_M_archives[index];
	return p;
}
