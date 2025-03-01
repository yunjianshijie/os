#ifndef __FS_FS_H
#define __FS_FS_H

#include "ide.h"

#define MAX_FILES_PER_RART 4096
// 每个分区所支持最大创建的文件数

#define BITS_PER_SECTOR 4096   // 每扇区的位数
#define SECTOR_SIZE 512        // 每扇区大小
#define BLOCK_SIZE SECTOR_SIZE // 每个块的大小
#define MAX_PATH_LEN 512       // 文件路径最大长度
/* 文件类型*/
enum file_types {
  FT_UNKNOWN,   //  不支持的文件类型
  FT_REGULAR,   // 普通文件
  FT_DIRECTORY, // 目录
};
/* 打开文件的选项 */
enum oflags {
  O_RDONLY,   // 只读
  O_WRONLY,   // 只写
  O_RDWR,     // 读写
  O_CREAT = 4 // 创建
};

/* 用来记录查找文件过程中已找到的上级路径，也就是查找文件过程中“走过的地方” */
struct path_search_record {
  char searched_path[MAX_PATH_LEN]; // 查找过程中的父路径
  struct dir *parent_dir;           // 文件或目录所在的直接父目录
  enum file_types
      file_type; // 找到的是普通文件，还是目录，找不到将为未知类型(FT_UNKNOWN)
};

// /*  格式化分区，也就是初始化分区的元信息，创建文件系统 */
static void partition_format(struct partition *part);
/* 在磁盘上搜索文件系统，若没有则格式化分区创建文件系统 */
void filesys_init(void);

// /* 在分区链表中找到名为 part_name 的分区，并将其指针赋值给 cur_part */
static bool mount_partition(struct list_elem *pelem, int arg);

static char *path_parse(char *pathname, char *name_store);
/* 打开或创建文件成功后，返回文件描述符，否则返回-1 */
int32_t sys_open(const char *pathname, uint8_t flags);
int32_t path_depth_cnt(char *pathname);
/*将 buf 中连续 count 个字节写入文件描述符 fd， 成功则返回写入的字节数，失败返回
 * -1 */
int32_t sys_write(int32_t fd, const void *buf, uint32_t count);
#endif