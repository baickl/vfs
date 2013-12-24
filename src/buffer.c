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
#include "buffer.h"
#include "pool.h"

typedef struct vfs_buffer_data
{
    VFS_SIZE _M_size;
    VFS_VOID*  _M_buf;
}vfs_buffer_data;


static VFS_VOID vfs_buffer_cleanup(vfs_buffer* obj )
{
    vfs_buffer_data* buf;

    if( !obj )
        return ;

    buf = obj->pThis;
    vfs_pool_free(buf->_M_buf);
    buf->_M_buf = 0;
}

static VFS_BOOL vfs_buffer_resize(vfs_buffer* obj ,VFS_SIZE size)
{
    VFS_VOID*p;
    vfs_buffer_data* buf;

    if( !obj )
        return VFS_FALSE;

    buf = obj->pThis;

    if( buf->_M_size == size )
        return VFS_TRUE;

    if( buf->_M_size == 0 )
    {
        buf->_M_buf = (VFS_VOID*)vfs_pool_malloc(size);
        if( buf->_M_buf )
        {
            buf->_M_size += size;
            return VFS_TRUE;
        }

        return VFS_FALSE;
    }
    else
    {
        if( size < buf->_M_size )
        {
            buf->_M_size = size;
            if( size == 0 )
            {
                vfs_pool_free(buf->_M_buf);
                buf->_M_buf = 0;
                return VFS_TRUE;
            }
            return VFS_TRUE;
        }
        else
        {

            p = (VFS_VOID*)vfs_pool_realloc(buf->_M_buf,size);
            if( !p )
                return VFS_FALSE;

            buf->_M_buf = p;
            buf->_M_size = size;
            return VFS_TRUE;
        }
    }
}


static VFS_SIZE vfs_buffer_get_size(vfs_buffer* obj)
{
    vfs_buffer_data* buf;

    if( !obj )
        return 0;

    buf = obj->pThis;
    return buf->_M_size;
}

static VFS_VOID* vfs_buffer_get_data(vfs_buffer*obj)
{
    vfs_buffer_data* buf;

    if( !obj )
        return 0;

    buf = obj->pThis;
    return buf->_M_buf;
}


vfs_buffer* vfs_buffer_new(VFS_SIZE size )
{
    vfs_buffer *obj;
    vfs_buffer_data*buf;
    VFS_VOID* p;

    obj = (vfs_buffer*)vfs_pool_malloc(size);
    if( obj == NULL )
        return NULL;

    obj->cleanup  = vfs_buffer_cleanup;
    obj->resize   = vfs_buffer_resize;
    obj->get_size = vfs_buffer_get_size;
    obj->get_data = vfs_buffer_get_data;

    buf = (vfs_buffer_data*)obj->pThis;
    buf->_M_size = 0;
    buf->_M_buf = 0;
    

    if( size !=  0 )
    {
        p = (VFS_VOID*)vfs_pool_malloc(size);
        if( p == NULL )
        {
            vfs_buffer_delete(obj);
            return NULL;
        }

        buf->_M_size = size;
        buf->_M_buf = p;
    }

    return obj;
}

VFS_VOID vfs_buffer_delete( vfs_buffer* obj )
{
    vfs_buffer_data *buf;
    if(!obj)
        return;
    
    buf = (vfs_buffer_data *)obj->pThis;
    buf->_M_size = 0;
    if( buf->_M_buf ){
        vfs_pool_free(buf->_M_buf);
        buf->_M_buf = NULL;
    }    

    vfs_pool_free(obj);
}

