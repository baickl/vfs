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

