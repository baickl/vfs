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
#ifndef _VFS_UITL_H_
#define _VFS_UTIL_H_

#include "base.h"

typedef var32 (*dir_foreach_item_proc)(const char*fullpath,var32 dir,void*p);

VFS_EXTERN uvar32			vfs_util_calc_crc32( void*, var32 );

VFS_EXTERN var32			vfs_util_compress_bound( var8 ,var32 );
VFS_EXTERN uvar64			vfs_util_compress( var32,  const void*, uvar64,void*, uvar64 );
VFS_EXTERN uvar64			vfs_util_decompress( var32, const void*, uvar64,void*, uvar64 );

VFS_EXTERN VFS_BOOL			vfs_util_dir_foreach(const char*,dir_foreach_item_proc,void*p);

VFS_EXTERN char*            vfs_util_str_toupper(char*);
VFS_EXTERN char*            vfs_util_str_tolower(char*);

VFS_EXTERN char*			vfs_util_path_checkfix(char*);
VFS_EXTERN char*			vfs_util_path_clone(char*,char*);
VFS_EXTERN char*			vfs_util_path_append(char*,char*);
VFS_EXTERN char*			vfs_util_path_join(char*,char*);
VFS_EXTERN char*			vfs_util_path_combine(char*,const char*,const char*);
VFS_EXTERN char*			vfs_util_path_add_backslash(char*);
VFS_EXTERN char*			vfs_util_path_remove_backslash(char*);
VFS_EXTERN char*			vfs_util_path_remove_filename(char*);
VFS_EXTERN char*			vfs_util_path_remove_extension(char*);

#endif/* _VFS_UTIL_H_ */
