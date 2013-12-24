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
#ifndef _VFS_PAK_H_
#define _VFS_PAK_H_

#include "pak_private.h"

/*****************************************************************************
 * pak_item_foreach_proc
   
 * @name                    pak_item_foreach_proc
 * @param   _pak            当前正在遍历的pak对象指针
 * @param   filename        当前遍历到的文件名
 * @param   iteminfo        当前遍历到的文件信息 
 * @param   index           当前遍历的文件索引
 * @param   p               遍历操作传入的参数，可以是一个结构指针，也可以是一个函数，用在函数里处理你的逻辑
 * @return  VFS_INT32       -VFS_FOREACH_IGNORE         返回FOREACH，本次结果不处理，如果是遇到目录返回，则表示不进入目录继续FOREACH
 *                          -VFS_FOREACH_CONTINUE       继续FOREACH，主要用于处理目录进入
 *                          -VFS_FOREACH_BREAK          中断FOREACH，一般是找到想要的文件夹或是文件后，不需要继续FOREACH
 *                          -VFS_FOREACH_PROC_ERROR      处理函数返回错误，要求中断FOREACH行为
 */

typedef VFS_INT32           (*pak_item_foreach_proc)(pak*_pak ,VFS_CHAR* filename, pak_iteminfo *iteminfo , int index , VFS_VOID *p );

/*****************************************************************************
 * 打开PAK文件包
   
 * @name                    pak_open
 * @param   pakfile         当前要打开的PAK文件
 * @param   prefix          给文件包里的文件加上前缀，此参数主要目的是配合相对路径来处理 
 * @return  pak*            打开pak成功，返回其结构实例，失败返回空指针
 * 
 * 解释一下本函数的第二个参数，假如我们开发模式下，以相对路径读取目录下的文件，
 * 文件路径如下：
 *      a/b/scene/scene0.sce
 *
 * 现在需要打包发行，但是我们希望将scene单独打包，b目录下的其他文件也单独打包，先来解决一下
 * scene目录打包的问题。
 *
 * 我们将scene目录打包以后，scene.pak的文件列表如下：
 * scene/scene0.sce
 * scene/scene1.sce
 * .
 * .
 * scenesceneN.sce
 *
 * 我们程序中用的是相对路径来读取，那如果我们将scene.pak直接读取不加任何前置路径来修正的话，
 * 我们的代码里所有
 * a/b/scene/sceneX.sce的都要修改成scene/sceneX.sce
 *
 * 这样违背了我们的初衷，我们希望打包以后，还能和以前一样做文件索引，所以我们在打开pak的时候，
 * 加上预留接口，可以控制文件索引按照我们希望的方式组合。
 *
 * 我们加上prefix参数，我们可以用这个参数来控制文件索引的建立，而不用再担心pak的位置。
 * 所以这个时候我们可以放心的加载,只需要在加载的时候告诉接口，我需要相对路径为a/b就OK了
 *
 * 然后我们建立文件索引的时候，我们会就在scene/sceneX.sce前面加上a/b/ 形成一个相对路径
 * 而且我们之前写过的代码不用做任何调整，就能生效，这样岂不是很方便？
 *
 */
VFS_EXTERN pak*             pak_open( const VFS_CHAR *pakfile,const VFS_CHAR *prefix );

/*****************************************************************************
 * 关闭PAK文件包
   
 * @name                    pak_close
 * @param   _pak            要关闭的pak对象
 * @return  VFS_VOID            无
 *
 */
VFS_EXTERN VFS_VOID         pak_close( pak* _pak );


/*****************************************************************************
 * 取得PAK中元素的个数
   
 * @name                    pak_item_get_count
 * @param   _pak            从哪个PAK对象中获取元素个数
 * @return  VFS_INT32           返回要获取的PAK对象中，元素个数 
 *
 */
VFS_EXTERN VFS_INT32        pak_item_get_count( pak*_pak );


/*****************************************************************************
 * 遍历PAK中的元素
   
 * @name                    pak_item_foreach
 * @param   _pak            待遍历的PAK对象
 * @param   proc            遍历到元素时的回调函数
 * @param   p               传递给回调函数的参数
 * @return  VFS_BOOL        ==VFS_TRUE 遍历成功完成
 *                          ==VFS_FALSE 遍历被中断或是遍历失败，如果回调函数返回FOREACH_PROC_ERROR那么也返回VFS_FALSE
 *
 */
VFS_EXTERN VFS_BOOL         pak_item_foreach( pak* _pak,pak_item_foreach_proc proc,VFS_VOID*p);


/*****************************************************************************
 * 在PAK中定位元素
   
 * @name                    pak_item_locate
 * @param   _pak            目标_pak,将从这个pak里定位
 * @param   file            要定位的文件
 * @return  pak_iteminf*    如果定位成功，返回包含文件信息的结构
 *                          ==NULL  如果没有定位到，返回空
 *
 */
VFS_EXTERN pak_iteminfo*    pak_item_locate( pak* _pak, const VFS_CHAR *file);


/*****************************************************************************
 * 在PAK中解压指定的文件到内存中
   
 * @name                    pak_item_unpack_filename
 * @param   _pak            目标_pak,将从这个pak里解包文件
 * @param   file            要解出来的文件
 * @param   buf             输出BUF
 * @param   bufsize         输出BUF的大小
 * @return  VFS_BOOL        ==VFS_TRUE   解压文件成功
 *                          ==VFS_FALSE  解压文件失败 
 *
 */
VFS_EXTERN VFS_BOOL         pak_item_unpack_filename( pak*_pak, const VFS_CHAR*file, VFS_VOID*buf, VFS_UINT64 bufsize) ;


#endif/*_VFS_PAK_H_*/
