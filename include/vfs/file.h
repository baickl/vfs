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
/* VFS文件结构                                                          */
/************************************************************************/
typedef struct vfs_file_t
{
	uvar64		_M_size;
	uvar64		_M_position;
	void*		_M_buffer;
}vfs_file;


/*****************************************************************************
 * 检查文件是否存在
   
 * @name                vfs_file_exists
 * @param   filename    待检测是否存在的文件
 * @return  var32       -VFS_FILE_NOT_EXISTS        文件不存在
 *                      -VFS_FILE_EXISTS_IN_PAK     文件存在于当前已经加载的PAK中
 *                      -VFS_FILE_EXISTS_IN_DIR     文件存在于当前程序运行的目录中
 *
 */
VFS_EXTERN var32        vfs_file_exists( const char *filename );

/*****************************************************************************
 * 创建一个读写的虚拟文件,可以指定初始化数据和大小
   
 * @name                vfs_file_create
 * @param   buf         用来初始化虚拟文件的数据
 * @param   bufsize     用来初始化虚拟文件的数据大小
 * @return  vfs_file*   成功返回vfs_file对象
 *                      失败返回NULL
 *
 */
VFS_EXTERN vfs_file*    vfs_file_create( void *buf , uvar64 bufsize) ;

/*****************************************************************************
 * 打开文件
   
 * @name                vfs_file_open
 * @param   filename    要打开的文件名
 * @return  vfs_file*   成功返回vfs_file对象
 *                      失败返回NULL
 *
 * 当打开的文件在PAK包和目录中都同时存在时，优先从PAK包中读取，
 * 如果PAK包中没有，仅在目录中存在，那么将从目录中优先加载。
 *
 * 总之，PAK包的优先级大于目录，索引文件也是优先从PAK包中索引，
 * 然后才会是从目录中索引。
 *
 */
VFS_EXTERN vfs_file*    vfs_file_open( const char *filename );

/*****************************************************************************
 * 关闭文件
   
 * @name                vfs_file_close
 * @param   _file       要关闭的文件指针对象
 * @return  void        无返回值 
 *
 */
VFS_EXTERN void         vfs_file_close( vfs_file *_file );

/*****************************************************************************
 * 保存文件
   
 * @name                vfs_file_save
 * @param   _file       要保存的文件指针对象
 * @param   filename    目标文件名称
 * @return  VFS_BOOL    返回VFS_TRUE   保存文件成功
 *                      返回VFS_FALSE  保存文件失败
 *
 */
VFS_EXTERN VFS_BOOL     vfs_file_save( vfs_file  *_file ,const char *filename);

/*****************************************************************************
 * 判断是否已到文件尾部 
   
 * @name                vfs_file_eof
 * @param   _file       文件指针对象
 * @return  VFS_BOOL    返回VFS_TRUE   已经抵达文件尾部
 *                      返回VFS_FALSE  没有抵达文件尾部
 *
 */
VFS_EXTERN VFS_BOOL     vfs_file_eof( vfs_file *_file );

/*****************************************************************************
 * 获取当前文件位移量 
   
 * @name                vfs_file_tell
 * @param   _file       文件指针对象
 * @return  uvar64      返回当前文件的位移量
 *
 */
VFS_EXTERN uvar64       vfs_file_tell( vfs_file *_file );

/*****************************************************************************
 * 移动到文件的指定位置 
   
 * @name                vfs_file_seek
 * @param   _file       文件指针对象
 * @param   pos         要移动的位移量
 * @param   mod         位移模式，主要有三种模式
 *                      =SEEK_SET   从文件头开始位移
 *                      =SEEK_CUR   从文件当前位置开始移动
 *                      =SEEK_END   从文件尾部开始移动
 * @return  uvar64      返回当前文件的位移量
 *
 */
VFS_EXTERN uvar64       vfs_file_seek( vfs_file *_file , var64 pos , var32 mod );

/*****************************************************************************
 * 获得文件的大小 
   
 * @name                vfs_file_size
 * @param   _file       文件指针对象
 * @return  uvar64      返回当前文件的大小
 *
 */
VFS_EXTERN uvar64       vfs_file_size( vfs_file *_file );

/*****************************************************************************
 * 获得文件的数据 
   
 * @name                vfs_file_data
 * @param   _file       文件指针对象
 * @return  const void* 返回当前文件的数据
 *
 */
VFS_EXTERN const void*  vfs_file_data( vfs_file *_file );

/*****************************************************************************
 * 读取文件数据 
   
 * @name                vfs_file_read
 * @param   buf         存放读出来的数据BUF
 * @param   size        要读的数据块的大小
 * @param   count       总共要读取多少数据块
 * @param   _file       文件指针对象
 * @return  size_t      返回实际读取了多少个数据块
 *
 */
VFS_EXTERN size_t       vfs_file_read( void *buf , size_t size , size_t count , vfs_file *_file);

/*****************************************************************************
 * 写入文件数据 
   
 * @name                vfs_file_write
 * @param   buf         存放准备写入文件的数据BUF
 * @param   size        要写的数据块的大小
 * @param   count       总共要写入多少数据块
 * @param   _file       文件指针对象
 * @return  size_t      返回实际写入了多少个数据块
 *
 */
VFS_EXTERN size_t       vfs_file_write( void *buf , size_t size, size_t count, vfs_file *_file);

#endif/*_VFS_FILE_H_*/
