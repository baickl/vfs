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
#ifndef _VFS_FILE_H_
#define _VFS_FILE_H_

#include "base.h"

/************************************************************************/
/* VFS文件结构                                                  */
/************************************************************************/
typedef struct vfs_file_t
{
	uvar64		_M_size;
	uvar64		_M_position;
	void*		_M_buffer;
}vfs_file;

/************************************************************************/
/* VFS系统中，文件的读写                                        */
/************************************************************************/
VFS_EXTERN var32            vfs_file_exists( const char* );

VFS_EXTERN vfs_file*		vfs_file_create( void*buf , uvar64 size) ;
VFS_EXTERN vfs_file*		vfs_file_open( const char* );
VFS_EXTERN void				vfs_file_close( vfs_file* );
VFS_EXTERN VFS_BOOL			vfs_file_save( vfs_file*,const char*);

VFS_EXTERN VFS_BOOL			vfs_file_eof( vfs_file* );
VFS_EXTERN uvar64			vfs_file_tell( vfs_file* );
VFS_EXTERN uvar64			vfs_file_seek( vfs_file* , var64 , var32 );
VFS_EXTERN uvar64			vfs_file_size( vfs_file* );
VFS_EXTERN const void*      vfs_file_data( vfs_file* );

VFS_EXTERN size_t			vfs_file_read( void* , size_t , size_t , vfs_file*);
VFS_EXTERN size_t			vfs_file_write( void* , size_t , size_t , vfs_file*);

#endif/*_VFS_FILE_H_*/
