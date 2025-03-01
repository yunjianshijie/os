#ifndef __FS_FILE_H
#define __FS_FILE_H
#include "fs.h"
#include "dir.h"
#include "global.h"
#include "inode.h"
#define MAX_FILE_OPEN 32 // 系统可打开的最大文件数
/* 文件结构 */
struct file {
   uint32_t fd_pos;
  // 记录当前文件操作的偏移地址，以 0 为起始，最大为文件大小-1
   uint32_t fd_flag;
   struct inode *fd_inode;
};
/* 标准输入输出描述符 */
 enum std_fd {
   stdin_no, // 0 标准输入
   stdout_no, // 1 标准输出
   stderr_no // 2 标准错误
 };

 /* 位图类型 */
 enum bitmap_type
 {
     INODE_BITMAP, // inode 位图
     BLOCK_BITMAP, // 块位图[]
 };
 int32_t block_bitmap_alloc(struct partition *part); // 分配一个磁盘块

 void bitmap_sync(struct partition *part, uint32_t bit_idx, uint8_t btmp);
 /*创建文件，若成功则返回文件描述符，否则返回-1 */
 int32_t file_create(struct dir *parent_dir, char *filename, uint8_t flag);
#endif
