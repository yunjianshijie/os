#ifndef __FS_FILE_H
#define __FS_FILE_H
#include "dir.h"
#include "fs.h"
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
  stdin_no,  // 0 标准输入
  stdout_no, // 1 标准输出
  stderr_no  // 2 标准错误
};

/* 位图类型 */
enum bitmap_type {
  INODE_BITMAP, // inode 位图
  BLOCK_BITMAP, // 块位图[]
};
int32_t block_bitmap_alloc(struct partition *part); // 分配一个磁盘块

void bitmap_sync(struct partition *part, uint32_t bit_idx, uint8_t btmp);
/*创建文件，若成功则返回文件描述符，否则返回-1 */
int32_t file_create(struct dir *parent_dir, char *filename, uint8_t flag);
/* 关闭文件描述符 fd 指向的文件，成功返回 0，否则返回-1 */
int32_t sys_close(int32_t fd);
/*打开编号为 inode_no 的 inode 对应的文件，
若成功则返回文件描述符，否则返回-1 */
int32_t file_open(uint32_t inode_no, uint8_t flag);
/* 关闭文件 */
int32_t file_close(struct file *file);

/* 把 buf 中的 count 个字节写入 file，
成功则返回写入的字节数，失败则返回-1 */
int32_t file_write(struct file *file, const void *buf, uint32_t count);
#endif
