#ifndef _VFS_HPP_
#define _VFS_HPP_

#include "vfs.h"
#include "file.h"


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
        if(VFS_TRUE == vfs_create(workpath))
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
        if( !file)
            return false;

        int count = vfs_get_pak_count();
        for( int i = 0; i<count; ++i )
        {
            pak* _pak = vfs_get_pak_index(i);
            if( _pak && pak_item_locate(_pak,file) >= 0  )
                return true;
        }

        return false;
    }
};

#define sglVFS VFS::GetInstance()

#endif//_VFS_HPP_
