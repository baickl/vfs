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
#ifndef _VFS_BASE_H_
#define _VFS_BASE_H_

#include "typedef.h"
#include "version.h"
#include <stdlib.h>
#include <string.h>

/************************************************************************/
/* 函数导出定义 */
#ifndef __cplusplus
#define VFS_EXTERN extern
#else
#define VFS_EXTERN extern "C"
#endif

/************************************************************************/
/* 版本定义 */

#define VFS_MAX_FILENAME                            (512)

#define VFS_FOREACH_IGNORE                          0	  /* 返回FOREACH，本次结果不处理，如果是遇到目录返回，则表示不进入目录继续FOREACH */
#define VFS_FOREACH_CONTINUE                        1     /* 继续FOREACH，主要用于处理目录进入*/
#define VFS_FOREACH_BREAK                           2     /* 中断FOREACH，一般是找到想要的文件夹或是文件后，不需要继续FOREACH了 */
#define VFS_FOREACH_PROC_ERROR                      3     /* 处理函数返回错误，要求中断FOREACH行为 */

/************************************************************************/
/* 压缩类型定义 */
#define VFS_COMPRESS_NONE                           (0x0000)
#define VFS_COMPRESS_BZIP2                          (0x0001)

/************************************************************************/
/* 检查文件存在的结果 */
#define VFS_FILE_NOT_EXISTS	                        (0)
#define VFS_FILE_EXISTS_IN_ARCHIVE                  (1)
#define VFS_FILE_EXISTS_IN_DIR                      (2)


/************************************************************************/
/* MAKE_CC  */
#define MAKE_CC_ID(c0, c1, c2, c3)                                      \
        ((uvar32)(uvar8)(c0) | ((uvar32)(uvar8)(c1) << 8) |             \
        ((uvar32)(uvar8)(c2) << 16) | ((uvar32)(uvar8)(c3) << 24 ))


/************************************************************************/
/* 相关实用宏定义 */
#define VFS_SAFE_FCLOSE(x)                          if(x){fclose(x);x=NULL;}
#define VFS_SAFE_FREE(x)                            if(x){free((void*)x);x=NULL;}

#define VFS_CHECK_FREAD(f,b,l)                      (f ? (fread((void*)b,1,(size_t)l,f)        != l ? 0 : 1) : 0 )
#define VFS_CHECK_FWRITE(f,b,l)                     (f ? (fwrite((const void*)b,1,(size_t)l,f) != l ? 0 : 1) : 0 )

#ifdef __linux__
    #define VFS_FSEEK                               fseeko
    #define VFS_FTELL                               ftello
#else
    #define VFS_FSEEK                               _fseeki64
    #define VFS_FTELL                               _ftelli64
#endif 

#endif/*_VFS_BASE_H_*/
