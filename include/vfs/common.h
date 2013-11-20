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
#ifndef _VFS_COMMON_H_
#define _VFS_COMMON_H_

#include "typedef.h"

#define VFS_EXTERN extern

#define VFS_SAFE_FCLOSE(x)		if(x){fclose(x);x=NULL;}
#define VFS_SAFE_FREE(x)		if(x){free((void*)x);x=NULL;}

#ifndef _WIN32 
	#define VFS_FSEEK	fseeko
	#define VFS_FTELL	ftello
	#define stricmp		strcasecmp
#else
	#define VFS_FSEEK	_fseeki64
	#define VFS_FTELL	_ftelli64
#endif 


#define VFS_CHECK_FREAD(f,b,l)	(f ? (fread((void*)b,1,(size_t)l,f)        != l ? 0 : 1) : 0 )
#define VFS_CHECK_FWRITE(f,b,l) (f ? (fwrite((const void*)b,1,(size_t)l,f) != l ? 0 : 1) : 0 )

#define VFS_VERSION				(1)

#define VFS_MAX_FILENAME		(512)

#define VFS_COMPRESS_NONE		(0x0000)
#define VFS_COMPRESS_BZIP2		(0x0001)

#define VFS_BOOL				int
#define VFS_TRUE				(1)
#define VFS_FALSE				(0)


#endif/*_VFS_COMMON_H_*/
