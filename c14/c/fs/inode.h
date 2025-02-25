#ifndef __FS_INODE_H
#define __FS_INODE_H
#include "global.h"
#include "list.h"
/* inode 结构 */
struct inode
{
    uint32_t i_no; // inode 编号
    /* 当此 inode 是文件时，i_size 是指文件大小,
     若此 inode 是目录，i_size 是指该目录下所有目录项大小之和*/
    uint32_t i_size;

    uint32_t i_open_cnts; // 打开此 inode 的进程数
    bool write_deny;      // 写文件不能并行，进程写文件前检查此标识

    /* i_sectors[0-11]是直接块，i_sectors[12]用来存储一级间接块指针 */
    uint32_t i_sectors[13];
    struct list_elem inode_tag;
};
/* 关闭inode或减少inode的打开数 */
void inode_close(struct inode *inode);
/* 初始化new_inode */
void inode_init(uint32_t inode_no, struct inode *new_inode);
/* 根据i结点号返回相应的i结点 */
struct inode *inode_open(struct partition *part, uint32_t inode_no);
/* 将inode写入到分区part */
void inode_sync(struct partition *part, struct inode *inode, void *io_buf);
/* 获取inode所在的扇区和扇区内的偏移量 */
static void inode_locate(struct partition *part, uint32_t inode_no,
                         struct inode_position *inode_pos);
#endif