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

/*****************************************************************************
 * 遍历目录的处理函数
   
 * @name                dir_foreach_item_proc
 * @param     fullpath  当前遍历时找到的文件或目录
 * @param     dir       如果dir==1,表示当前项目是目录，否则表示当前项目文件
 * @param     p         上层传递进来的参数，可以借此做一些扩展处理
 * @return    uvar32    返回FOREACH_IGNOR,通过遍历函数，不递归当前的目录遍历，不会对遍历其他目录的处理有影响
 *                      返回FOREACH_CONTINUE,通知遍历函数，继续遍历
 *                      返回FOREACH_BREAK,通知遍历函数，中断遍历
 *                      返回FOREACH_PROC_ERROR,通知遍历函数，处理函数处理时有错误，中断遍历
 *                      
 */
typedef var32           (*dir_foreach_item_proc)(const char*fullpath,var32 dir,void*p);

/*****************************************************************************
 * 计算一段内存的校验码
   
 * @name                vfs_util_calc_crc32
 * @param     buf       输入数据
 * @param     bufsize   输入数据大小
 * @return    uvar32    返回计算出来的校验码
 *                      
 */
VFS_EXTERN uvar32		vfs_util_calc_crc32( void *buf, var32 bufsize );

/*****************************************************************************
 * 输入原始数所大小和采用的算法，计算出压缩时需要的最大缓存大小
   
 * @name                vfs_util_compress_bound
 * @param     type      压缩方法
 * @param     len       原始数据长度
 * @return    uvar32    返回压缩时需要的缓存的最大长度
 *                      
 */
VFS_EXTERN var32		vfs_util_compress_bound( var8 type,var32 len );

/*****************************************************************************
 * 根据指定算法压缩一段数据
   
 * @name                vfs_util_compress
 * @param     type      采用的压缩类型
 * @param     src       待压缩的数据
 * @param     srclen    待压缩的数据长度
 * @param     dst       存放压缩后的数据缓存
 * @param     dstlen    存放压缩后数据缓存的长度
 * @return    uvar64    返回压缩后的数据长度，这个长度可能会比压缩前的长度大
 *                      
 */
VFS_EXTERN uvar64		vfs_util_compress(   var32 type , const void *src, uvar64 srclen ,void *dst, uvar64 dstlen);

/*****************************************************************************
 * 根据指定算法解压缩一段数据
   
 * @name                vfs_util_decompress
 * @param     type      采用的解压缩类型
 * @param     src       待解压的数据
 * @param     srclen    待解压的数据长度
 * @param     dst       存放解压后的数据缓存
 * @param     dstlen    存放解压后数据缓存的长度
 * @return    uvar64    返回解压后的数据长度，这个长度不会超过输出缓存大小
 *                      
 */
VFS_EXTERN uvar64		vfs_util_decompress( var32 type , const void *src, uvar64 srclen ,void *dst, uvar64 dstlen);

/*****************************************************************************
 * 遍历目录
   
 * @name                vfs_util_dir_foreach
 * @param     dir       要遍历的目录，形如:/home/xxx/app/或C:\\window\
 * @param     proc      遍历过程中，对当前的文件或是目录行处理的回调函数
 * @param     p         传递给proc的参数，上层接口可以传一个参数给proc
 * @return    VFS_BOOL  返回VFS_TRUE    遍历目录成功
 *                      返回VFS_FALSE   遍历目录失败
 *                      
 */
VFS_EXTERN VFS_BOOL		vfs_util_dir_foreach(const char* dir, dir_foreach_item_proc proc,void *p);

/*****************************************************************************
 * 将字符串里的小写字母转换成大写
   
 * @name                vfs_util_str_toupper
 * @param     str       待转换的字符串 
 * @return    char*     返回转换后的字符串str
 *
 * 此函数会修改传入的参数str
 *                      
 */
VFS_EXTERN char*        vfs_util_str_toupper(char *str);

/*****************************************************************************
 * 将字符串里的大写字母转换成小写
   
 * @name                vfs_util_str_tolower
 * @param     str       待转换的字符串 
 * @return    char*     返回转换后的字符串str
 *                      
 * 此函数会修改传入的参数str
 */
VFS_EXTERN char*        vfs_util_str_tolower(char *str);

/*****************************************************************************
 * 检查并修正路径
   
 * @name                vfs_util_path_checkfix
 * @param     path      待检查和修正的路径
 * @return    char*     返回修更后的path
 *
 * 此函数会修改传进去的path,并且会将目录中的'\\'转换成'/'
 *                      
 */
VFS_EXTERN char*		vfs_util_path_checkfix(char *path);

/*****************************************************************************
 * 克隆一个路径
   
 * @name                vfs_util_path_clone
 * @param     path      克隆路径的输出
 * @param     clone     待克隆的路径
 * @return    char*     返回克隆的path
 *
 * 此函数会先拷贝要克隆的对象，然后调用vfs_util_path_checkfix对拷贝出来的数据进行检查修正
 *                      
 */
VFS_EXTERN char*		vfs_util_path_clone(char *path,char *clone);

/*****************************************************************************
 * 路径追加
   
 * @name                vfs_util_path_append
 * @param     path      要追加路径，并存放追加结果,需要保证这个缓存有足够大小放置追加部分的数据
 * @param     append    待追加的路径
 * @return    char*     返回追加结果即path
 *
 * 如果path没有以'/'结尾，那么和append组合的时候，会在中间添加'/'
 * 如果path原本就以'/'结尾，那么,path和append组合后不添加任何东西
 *
 * 此函数使用方法如下：
 * char path[VFS_MAX_FILENAME];
 *
 * strcpy(path,"/home/xxx/app")
 * vfs_util_path_append(path,"aaa/bin.txt");
 * printf(path); 
 *
 * 输出结果：
 * /home/xxx/app/aaa/bin.txt
 *
 */
VFS_EXTERN char*		vfs_util_path_append(char *path,char *append);

/*****************************************************************************
 * 路径连接
   
 * @name                vfs_util_path_join
 * @param     path      要连接路径，并存放连接结果,需要保证这个缓存有足够大小放置连接部分的数据
 * @param     join      待连接的部分
 * @return    char*     返回连接结果，即参数path
 *
 * 路径连接不会在path和join之间添加任何东西，只是单纯的连接
 *
 * 此函数使用方法如下：
 * char path[VFS_MAX_FILENAME];
 *
 * strcpy(path,"/home/xxx/app/bin")
 * vfs_util_path_append(path,".txt");
 * printf(path); 
 *
 * 输出结果：
 * /home/xxx/app/bin.txt
 *
 */
VFS_EXTERN char*		vfs_util_path_join(char*,char*);

/*****************************************************************************
 * 路径组合
   
 * @name                vfs_util_path_combine
 * @param     path      存放组合后的输出结果，需要保证足够大小
 * @param     a         待组合的路径前半部分a 
 * @param     b         待组合的路径后半部分b
 * @return    char*     返回组合结果，即参数path
 *
 * 路径组合的时候，如果a不是以'/'结尾，那么程序会在组合的时候加上'/'，然后再连接b 
 *
 * 此函数使用方法如下：
 * char path[VFS_MAX_FILENAME];
 *
 * vfs_util_path_combine(path,"/home/xxx/app","bin.txt");
 * printf(path); 
 *
 * 输出结果：
 * /home/xxx/app/bin.txt
 *
 */
VFS_EXTERN char*		vfs_util_path_combine(char *path,const char *a,const char *b);

/*****************************************************************************
 * 在路径后面添加斜杠
   
 * @name                vfs_util_path_add_backslash
 * @param     path      待添加的路径，需要保证足够大小
 * @return    char*     返回添加结果，即参数path
 *
 * 如果路径结尾已经以'/'结尾，那么本函数，不对path做任何改变，
 * 如果路径结尾不是'/'，那么会在路径结尾添加'/'，然后返回结果
 *
 * 此函数使用方法如下：
 * char path[VFS_MAX_FILENAME];
 * strcpy(path,"home/xxx/app");
 * vfs_util_path_add_backslash(path)
 * printf(path); 
 *
 * 输出结果：
 * /home/xxx/app/
 *
 */
VFS_EXTERN char*		vfs_util_path_add_backslash(char *path);

/*****************************************************************************
 * 去除路径后面的斜杠
   
 * @name                vfs_util_path_remove_backslash
 * @param     path      待处理的路径
 * @return    char*     返回添加结果，即参数path
 *
 * 如果路径结尾不是以'/'结尾，将不会做任何处理，并且直接返回
 * 如果路径结尾是以'/'结尾，，将会移除'/'，然后返回结果
 *
 * 此函数使用方法如下：
 * char path[VFS_MAX_FILENAME];
 * strcpy(path,"home/xxx/app/");
 * vfs_util_path_remove_backslash(path)
 * printf(path); 
 *
 * 输出结果：
 * /home/xxx/app
 *
 */
VFS_EXTERN char*		vfs_util_path_remove_backslash(char *path);

/*****************************************************************************
 * 移除路径中的文件名
   
 * @name                vfs_util_path_remove_filename
 * @param     path      待处理的路径
 * @return    char*     返回添加结果，即参数path
 *
 * 此函数使用方法如下：
 * char path[VFS_MAX_FILENAME];
 * strcpy(path,"home/xxx/app/bin.txt");
 * vfs_util_path_remove_filename(path)
 * printf(path); 
 *
 * 输出结果：
 * /home/xxx/app/
 *
 */
VFS_EXTERN char*		vfs_util_path_remove_filename(char *path);

/*****************************************************************************
 * 移除路径中的文件扩展名
   
 * @name                vfs_util_path_remove_extension
 * @param     path      待处理的路径
 * @return    char*     返回添加结果，即参数path
 *
 * 此函数使用方法如下：
 * char path[VFS_MAX_FILENAME];
 * strcpy(path,"home/xxx/app/bin.txt");
 * vfs_util_path_remove_extension(path)
 * printf(path); 
 *
 * 输出结果：
 * /home/xxx/app/bin
 *
 */
VFS_EXTERN char*		vfs_util_path_remove_extension(char*);

#endif/* _VFS_UTIL_H_ */
