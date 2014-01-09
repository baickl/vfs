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

#include "base.h"
#include "plugin.h"
#include "stream.h"

/*****************************************************************************
 * vfs系统创建
   
 * @name                    vfs_create
 * @param   version         sdk版本号
 * @param   workpath        程序的工作目录
 * @return  VFS_BOOL        返回VFS_TRUE   创建VFS系统成功
 *                          返回VFS_FALSE  创建VFS系统失败
 *
 * 此函数是创建vfsh虚拟文件系统的，需要在创建的时候，指定资源路径或是程序运行路径
 * 指定一个工作路径的目标是为了方便的文件加载模块可以通过相对路径来读取文件
 *
 * 举例：
 * 资源路径是/home/xxxx/app/a/b/c/data/logo.png
 * 程序路径是/home/xxxx/app
 *
 * 那么初始化的时候，要将/home/xxxx/app传给vfs_create,程序初始化成功后
 * 可以通过相对路径a/b/c/data/logo.png来加载资源
 *
 * 如果资源目录打包的话，只需要再将包添加到资源里就可以了
 * 比如说对data目录打包，那么添加包的代码就是
 * vfs_add_archive("a/b/c/data.pak")
 *
 * 以上代码，即使在开发时，也不会出错，因为开发时包不存在，VFS本身不会对这种文件包不存在的情况
 * 进行报错和预处理。
 *
 * 所以程序最终读取的时候，还是按照相对路径来读取，所以开发完成的时候，只需要将目录打包然后放到这个目录的同级目录
 * 即可,程序就将从包里读取数据而不再从目录中读取数据。
 * 
 *
 */
VFS_EXTERN VFS_BOOL         vfs_create( const VFS_CHAR *sdk_version,
                                        const VFS_CHAR *workpath);

/*****************************************************************************
 * vfs系统销毁
   
 * @name                    vfs_destroy
 * @return    VFS_VOID      无返回值
 *                      
 *
 * 销毁VFS系统，此函数不返回任何值，调用即销毁了，
 * 注意，销毁以后，之前通过vfs_add_archive加载的archive，将全部被清除
 * 此函数的调用时期，一般是程序要结束的时候，释放资源的时候调用
 */
VFS_EXTERN VFS_VOID         vfs_destroy();

/*****************************************************************************
 * 定位vfs插件
   
 * @name                    vfs_locate_plugin
 * @param     pluginname    插件名称
 * @return    vfs_plugin*   如果返回值不为空，定位插件成功
 *                          如果返回值为空，则没找到插件
 *
 */
VFS_EXTERN vfs_plugin*      vfs_locate_plugin(const VFS_CHAR* pluginname);

/*****************************************************************************
 * 注册vfs插件
   
 * @name                    vfs_register_plugin
 * @param     pluginname    插件名称
 * @param     plugin        待注册的插件
 * @return    VFS_BOOK      ==VFS_TRUE  注册成功
 *                          ==VFS_FALSE 注册插件失败  
 *
 */
VFS_EXTERN VFS_BOOL         vfs_register_plugin(const VFS_CHAR*pluginname,vfs_plugin plugin);

/*****************************************************************************
 * 反注册vfs插件
   
 * @name                    vfs_unregister_plugin
 * @param     pluginname    插件名称
 * @return    VFS_VOID      无返回值
 * 
 */
VFS_EXTERN VFS_VOID         vfs_unregister_plugin(const VFS_CHAR*pluginname );

/*****************************************************************************
 * 添加archive到vfs系统中
   
 * @name                    vfs_add_archive
 * @param   archive         待添加的archive文件
 * @param   passwd          打开archive用的密码，如果没有就置空
 * @return  VFS_BOOL        返回VFS_TRUE   添加archive成功
 *                          返回VFS_FALSE  添加archive失败
 *
 */
VFS_EXTERN VFS_BOOL         vfs_add_archive( const VFS_CHAR* archive,const VFS_CHAR* passwd );

/*****************************************************************************
 * 从VFS系统中移除archive文件
   
 * @name                    vfs_remove_archive
 * @param   archive         待移除的archive文件
 * @return  VFS_BOOL        返回VFS_TRUE   移除archive成功
 *                          返回VFS_FALSE  移除archive失败
 *
 */
VFS_EXTERN VFS_BOOL         vfs_remove_archive( const VFS_CHAR*archive );

/*****************************************************************************
 * 检查文件是否存在
   
 * @name                    vfs_file_exists
 * @param   filename        待检测是否存在的文件
 * @return  VFS_INT32       -VFS_FILE_NOT_EXISTS        文件不存在
 *                          -VFS_FILE_EXISTS_IN_ARCHIVE 文件存在于当前已经加载的ARCHIVE中
 *                          -VFS_FILE_EXISTS_IN_DIR     文件存在于当前程序运行的目录中
 *
 */
VFS_EXTERN VFS_INT32        vfs_file_exists( const VFS_CHAR *filename );


#endif
