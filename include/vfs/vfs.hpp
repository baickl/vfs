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
#ifndef _VFS_HPP_
#define _VFS_HPP_

#include "vfs.h"


///////////////////////////////////////////////////////////////////////////////
// 文件的简单封装
///////////////////////////////////////////////////////////////////////////////
class VFSFile
{
    VFSFile(const VFS&);
    VFSFile& operator=(const VFS&);

public:

    VFSFile()
        :_M_file(NULL)
    {
    }

    VFSFile( VFS_VOID*buf,VFS_SIZE bufsize )
        :_M_file(NULL)
    {
        Create(buf,bufsize);
    }

    VFSFile( const VFS_CHAR*file )
        :_M_file(NULL)
    {
        Open(file);
    }

    ~VFSFile()
    {
        Close();
    }

public:

    bool Create(VFS_VOID*buf,VFS_UINT64 bufsize)
    {
        Close();

        struct vfs_stream* stream = vfs_stream_create(buf,bufsize);
        if( !stream  )
        {
            return false;
        }

        _M_file = stream;
        return true;
    }

    bool Open(const VFS_CHAR* file )
    {
        Close();


        struct vfs_stream* stream = vfs_stream_open(file);
        if( !stream  )
        {
            return false;
        }

        _M_file = stream;

        return true;
    }

    VFS_VOID Close()
    {
        if( _M_file )
        {
            vfs_stream_close(_M_file);
            _M_file = NULL;
        }
    }

    bool Save(const VFS_CHAR* file )
    {
        if( !_M_file || !file )
            return false;

        return VFS_TRUE == vfs_stream_save(file);
    }

public:

    bool Eof()const 
    {
        if( !_M_file )
            return true;

        return vfs_stream_eof(_M_file);
    }

    VFS_UINT64 Tell()const 
    {
        if( !_M_file )
            return 0;

        return vfs_stream_tell(_M_file);
    }

    VFS_UINT64 Seek(VFS_UINT64 pos,int mod = SEEK_SET)
    {
        if( !_M_file )
            return 0;

        return vfs_stream_seek(_M_file,pos,mod);
    }

    VFS_UINT64 Size()const 
    {
        if( !_M_file )
            return 0;

        return vfs_stream_size(_M_file);
    }

    const VFS_VOID* Data()const
    {
        if( !_M_file )
            return 0;

        return vfs_stream_data(_M_file);
    }

    VFS_SIZE Read(VFS_VOID*buf,VFS_SIZE size,VFS_SIZE count )
    {
        if( !_M_file )
            return 0;

        return vfs_stream_read(_M_file,buf,size,count);
    }

    VFS_SIZE Write(VFS_VOID*buf,VFS_SIZE size,VFS_SIZE count )
    {
        if( !_M_file )
            return 0;

        return vfs_stream_write(_M_file,buf,size,count);
    }

private:

    struct vfs_stream *_M_file;
};


///////////////////////////////////////////////////////////////////////////////
// vfs wrapper
///////////////////////////////////////////////////////////////////////////////
class VFS
{
    VFS(){}
    VFS(const VFS&);
    VFS& operator=(const VFS&);

public:
    ~VFS()
	{
		Destroy();
	}

    static VFS& GetInstance(){
		static VFS Instance;
        return Instance;
    }

public:

    bool Create(const VFS_CHAR*workpath)
    {
        if(VFS_TRUE == vfs_create(VFS_SDK_VERSION,workpath))
            return true;
        else
            return false;
    }

    VFS_VOID Destroy()
    {
        vfs_destroy();
    }

    bool AddArchive(const VFS_CHAR* path )
    {
        if( VFS_TRUE == vfs_add_archive(path) )
            return true;
        else
            return false;
    }

    bool FileExists(const VFS_CHAR* file )
    {
        if( VFS_FILE_NOT_EXISTS != vfs_file_exists(file) )
            return true;
        return false;
    }
};


///////////////////////////////////////////////////////////////////////////////
// 简化宏的使用
#define sglVFS VFS::GetInstance()

#endif//_VFS_HPP_
