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

void vfs_buffer_init(struct vfs_buffer* buf )
{
    if( !buf )
        return;

    buf->_M_size = 0;
    buf->_M_buf = 0;
}

void* vfs_buffer_alloc(struct vfs_buffer* buf ,size_t size)
{
    void*p;

    if( !buf )
        return NULL;

    if( buf->_M_size == size )
        return buf->_M_buf;

    if( buf->_M_size == 0 ){
        buf->_M_buf = malloc(size);
        if( buf->_M_buf ){
            buf->_M_size += size;
            return buf->_M_buf;
        }
    }else{
        if( size < buf->_M_size ){
            buf->_M_size = size;
            if( size == 0 ){
                free(buf->_M_buf);
                buf->_M_buf = 0;
            }
            return buf->_M_buf;
        }else{

            p = malloc(size);
            if( !p )
                return NULL;

            memcpy(p,buf->_M_buf,buf->_M_size);
            free(buf->_M_buf);
            buf->_M_buf = p;
            buf->_M_size = size;
            return buf->_M_buf;
        }
    }
}

void vfs_mm_buffer_cleanup(struct vfs_buffer* buf)
{
    if(!buf)
        return;

    buf->_M_size = 0;
    if( buf->_M_buf ){
        free(buf->_M_buf);
        buf->_M_buf = NULL;
    }
}

