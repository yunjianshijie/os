#ifndef __FS_DIR_H
#define __FS_DIR_H

#include "fs.h"
#include "inode.h"
#include "super_block.h"
#define MAX_FILE_NAME_LEN 16 // 最大文件名长度

/* 目录结构*/
struct dir {
  struct inode *inode;  // 指向目录所在的inode
  uint32_t dir_pos;     // 当前目录读取的位置
  uint8_t dir_buf[512]; // 目录数据缓存区
};

/* 目录项结构 */
struct dir_entry {
  char filename[MAX_FILE_NAME_LEN]; // 文件名
  uint32_t i_no;                    // 普通文件或目录对应的inode编号
  enum file_types f_type;           // 文件类型
};
bool sync_dir_entry(struct dir *parent_dir, struct dir_entry *p_de,
                    void *io_buf);
bool search_dir_entry(struct partition *part, struct dir *pdir,
                      const char *name, struct dir_entry *dir_e);
/* 关闭目录 */
void dir_close(struct dir *dir);
/* 在分区part上打开i结点为indoe_no 的 目录并返回目录指针 */
struct dir *dir_open(struct partition *part, uint32_t inode_no);
/* 在内存中初始化目录项p_de*/
void create_dir_entry(char *filename, uint32_t inode_no, uint8_t file_type,
                      struct dir_entry *p_de);
/* 打开根目录 */
void open_root_dir(struct partition *part);
#endif