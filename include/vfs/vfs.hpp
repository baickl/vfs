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
#include "file.h"


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

    VFSFile( void*buf,size_t bufsize )
        :_M_file(NULL)
    {
        Create(buf,bufsize);
    }

    VFSFile( const char*file )
        :_M_file(NULL)
    {
        Open(file);
    }

    ~VFSFile()
    {
        Close();
    }

public:

    bool Create(void*buf,uvar64 bufsize)
    {
        Close();

        _M_file == vfs_file_create(buf,bufsize);
        if( _M_file )
            return true;
        else
            return false;
    }

    bool Open(const char* file )
    {
        Close();

        _M_file = vfs_file_open(file);
        if( _M_file )
            return true;
        else
            return false;
    }

    void Close()
    {
        if( _M_file )
        {
            vfs_file_close(_M_file);
            _M_file = NULL;
        }
    }

    bool Save(const char* file )
    {
        return vfs_file_save(file);
    }

public:

    bool Eof()const 
    {
        return vfs_file_eof(_M_file);
    }

    uvar64 Tell()const 
    {
        return vfs_file_tell(_M_file);
    }

    uvar64 Seek(uvar64 pos,int mod = SEEK_SET)
    {
        return vfs_file_seek(_M_file,pos,SEEK_SET);
    }

    uvar64 Size()const 
    {
        return vfs_file_size(_M_file);
    }

    const void* Data()const
    {
        return vfs_file_data(_M_file);
    }

    size_t Read(void*buf,size_t size,size_t count )
    {
        return vfs_file_read(buf,size,count,_M_file);
    }

    size_t Write(void*buf,size_t size,size_t count )
    {
        return vfs_file_write(buf,size,count,_M_file);
    }

private:

    vfs_file *_M_file;
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

    bool Create(const char*workpath)
    {
        if(VFS_TRUE == vfs_create(workpath,NULL))
            return true;
        else
            return false;
    }

    void Destroy()
    {
        vfs_destroy();
    }

    bool AddPath(const char* path )
    {
        if( VFS_TRUE == vfs_add_pak(path) )
            return true;
        else
            return false;
    }

    bool FileExists(const char* file )
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
