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
#ifndef _VFS_VFS_H_
#define _VFS_VFS_H_

#include "pak.h"

/* VFS  */
typedef struct vfs_t
{
	var32		_M_count;
	var32		_M_maxcount;
	pak**		_M_paks;
}vfs;

/* VFS_FILE */
typedef struct vfs_file_t
{
	uvar64		_M_size;
	uvar64		_M_position;
	void*		_M_buffer;
}vfs_file;

/* VFS系统 */
VFS_EXTERN VFS_BOOL			vfs_create();
VFS_EXTERN void				vfs_destroy();

VFS_EXTERN VFS_BOOL			vfs_pak_add(const char*);
VFS_EXTERN VFS_BOOL			vfs_pak_del(const char*);
VFS_EXTERN var32			vfs_pak_get_count();
VFS_EXTERN pak*				vfs_pak_get_index(var32);
VFS_EXTERN pak*				vfs_pak_get_name(const char*);


/* VFS对FILE相关函数模拟 */
VFS_EXTERN vfs_file*		vfs_fcreate(void*buf,uvar64 size);
VFS_EXTERN vfs_file*		vfs_fopen(const char*,const char*);
VFS_EXTERN void				vfs_fclose(vfs_file*);
VFS_EXTERN VFS_BOOL			vfs_feof(vfs_file*);
VFS_EXTERN uvar64			vfs_ftell(vfs_file*);
VFS_EXTERN uvar64			vfs_fseek(vfs_file*,var64,var32);
VFS_EXTERN uvar64			vfs_fsize(vfs_file*);
VFS_EXTERN uvar64			vfs_fread(void*,uvar64,uvar64,vfs_file*);
VFS_EXTERN uvar64			vfs_fwrite(void*,uvar64,uvar64,vfs_file*);
VFS_EXTERN VFS_BOOL			vfs_fflush(vfs_file*,const char*);

#endif
