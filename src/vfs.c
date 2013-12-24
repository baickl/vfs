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
#include <vfs/util.h>
#include "vfs_private.h"
#include "plugin/pak_plugin.h"
#include "plugin/bz2compress_plugin.h"
#include "pool.h"

#include <stdio.h>
#include <string.h>

vfs *g_vfs = NULL;


/************************************************************************/
/* pak search and locate                                                */
/************************************************************************/
static var32 vfs_archive_obj_sort_cmp(const void*a,const void*b)
{
	vfs_archive_obj* _a;
	vfs_archive_obj* _b;
	
	_a = *(vfs_archive_obj**)a;
	_b = *(vfs_archive_obj**)b;

	return stricmp(_a->plugin->info.get_plugin_name(_a->archive),
                   _b->plugin->info.get_plugin_name(_b->archive));
}

static var32 vfs_archive_obj_search_cmp(const void*key,const void*item)
{
	char*_key;
	vfs_archive_obj* _item;

	_key  = (char*)key;
	_item = *(vfs_archive_obj**)item;
	return stricmp(_key,_item->plugin->plugin.archive.archive_get_name(_item->archive));
}

static void vfs_archive_obj_sort()
{
	qsort((void*)g_vfs->_M_archives,g_vfs->_M_count,sizeof(vfs_archive_obj*),vfs_archive_obj_sort_cmp);
}

static var32 vfs_archive_obj_search(const char* archive)
{
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

    return stricmp(_a->info.get_plugin_name(),_b->info.get_plugin_name());
}

static var32 vfs_plugin_search_cmp(const void*key,const void*item)
{
    char*_key;
    vfs_plugin* _item;

    _key  = (char*)key;
    _item = *(vfs_plugin**)item;
    return stricmp(_key,_item->info.get_plugin_name());
}

static void vfs_plugin_sort()
{
    qsort((void*)g_vfs->_M_plugins,g_vfs->_M_plugins_count,sizeof(vfs_plugin*),vfs_plugin_sort_cmp);
}

static var32 vfs_plugin_search(const char* pluginname)
{
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
        if( plugin->type != PLUGIN_ARCHIVE)
            continue;

        if( VFS_TRUE == plugin->plugin.archive.archive_check_suppert(archive) )
            return plugin;
    }

    return NULL;
}

/************************************************************************/
/* vfs implement                                                        */
/************************************************************************/

VFS_BOOL vfs_create(const char* sdk_version,const char* workpath)
{
    vfs_plugin plugin;

	if( g_vfs )
		return VFS_TRUE;

    if( !workpath || !sdk_version )
        return VFS_FALSE;

    if( stricmp(VFS_SDK_VERSION,sdk_version) != 0 )
        return VFS_FALSE;

    if( VFS_FALSE == vfs_pool_init() )
        return VFS_FALSE;

	g_vfs = (vfs*)(vfs_pool_malloc(sizeof(vfs)));
	if( !g_vfs )return VFS_FALSE;

	g_vfs->_M_count = 0;
	g_vfs->_M_maxcount = 0;
	g_vfs->_M_archives = NULL;

    g_vfs->_M_plugins_count = 0;
    g_vfs->_M_plugins_maxcount = 0;
    g_vfs->_M_plugins = NULL;

    strcpy(g_vfs->_M_workpath,workpath);


    plugin = vfs_get_plugin_archive_pak();
    if( VFS_TRUE != vfs_register_plugin(plugin.info.get_plugin_name(),plugin) )
    {
        vfs_destroy();
        return VFS_FALSE;
    }

    plugin = vfs_get_plugin_compress_bz2compress();
    if( VFS_TRUE != vfs_register_plugin(plugin.info.get_plugin_name(),plugin) )
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
        archive->plugin->plugin.archive.archive_close(archive->archive);
	}

	if(g_vfs->_M_archives){
        vfs_pool_free(g_vfs->_M_archives);
        g_vfs->_M_archives = NULL;
    }

    if(g_vfs->_M_plugins){
        vfs_pool_free(g_vfs->_M_plugins);
        g_vfs->_M_plugins = NULL;
    }
	
    vfs_pool_free(g_vfs);
    g_vfs = NULL;

    vfs_pool_release();
}


vfs_plugin* vfs_locate_plugin(const char*pluginname)
{
    var32 index;
    
    if( !pluginname )
        return NULL;

    index = vfs_plugin_search(pluginname);
    if( index >= 0 && index < g_vfs->_M_plugins_count )
        return g_vfs->_M_plugins[index];
    else
        return NULL;
}


VFS_BOOL vfs_register_plugin(const char*pluginname,vfs_plugin plugin)
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
			g_vfs->_M_plugins = (vfs_plugin**)vfs_pool_malloc(g_vfs->_M_plugins_maxcount*sizeof(vfs_plugin*));
			if( !g_vfs->_M_plugins )
			{
				g_vfs->_M_plugins_maxcount = 0;
				return VFS_FALSE;
			}
		}
		else
		{
			g_vfs->_M_plugins_maxcount += 16;
			_plugins = (vfs_plugin**)vfs_pool_realloc(g_vfs->_M_plugins,g_vfs->_M_plugins_maxcount*sizeof(vfs_plugin*));
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

    _plugin = (vfs_plugin*)vfs_pool_malloc(sizeof(vfs_plugin));
    if( !_plugin )return VFS_FALSE;
    memcpy(_plugin,&plugin,sizeof(vfs_plugin));
    g_vfs->_M_plugins[g_vfs->_M_plugins_count++] = _plugin;

    return VFS_TRUE;
}

void vfs_unregister_plugin(const char*pluginname )
{
    var32 index;
    if( !g_vfs )
        return;

    if( !pluginname )
        return;

    index = vfs_plugin_search(pluginname);
    if( index < 0 || index >= g_vfs->_M_plugins_count )
        return;

    vfs_pool_free(g_vfs->_M_plugins[index]);
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

    if( !vfs_util_path_clone(_filepath,(char*)archive) )
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

    p = (vfs_archive_obj*)vfs_pool_malloc(sizeof(vfs_archive_obj));
    if( !p )
        return VFS_FALSE;

    p->plugin = plugin;
	p->archive = p->plugin->plugin.archive.archive_open(_fullpath,prefix,passwd);
	if( !p->archive)
    {
        vfs_pool_free(p);
        return VFS_FALSE;
    }

	if( g_vfs->_M_count >= g_vfs->_M_maxcount )
	{
		if( g_vfs->_M_count == 0 )
		{
			g_vfs->_M_maxcount = 16;
			g_vfs->_M_archives = (vfs_archive_obj**)vfs_pool_malloc(g_vfs->_M_maxcount*sizeof(vfs_archive_obj*));
			if( !g_vfs->_M_archives )
			{
				g_vfs->_M_maxcount = 0;
                p->plugin->plugin.archive.archive_close(p->archive);
                vfs_pool_free(p);

				return VFS_FALSE;
			}
		}
		else
		{
			g_vfs->_M_maxcount += 16;
			_archives = (vfs_archive_obj**)vfs_pool_realloc(g_vfs->_M_archives,g_vfs->_M_maxcount*sizeof(vfs_archive_obj*));
			if( !_archives )
			{
				g_vfs->_M_maxcount -= 16;
				
                p->plugin->plugin.archive.archive_close(p->archive);
                vfs_pool_free(p);

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
    p->plugin->plugin.archive.archive_close(p->archive);
	vfs_pool_free(p);

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
