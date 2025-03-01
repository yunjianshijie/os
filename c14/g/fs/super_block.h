#ifndef __FS_SUPER_BLOCK_H
#define __FS_SUPER_BLOCK_H
#include "global.h"
/* 超级块 */
struct super_block
{
    uint32_t magic;
    // 用来标识文件系统类型
    // 支持多文件系统的操作系统通过此标志来识别文件系统类型
    uint32_t sec_cnt;            // 本分区总共的扇区数
    uint32_t inode_cnt;          // 本分区中 inode 数量
    uint32_t part_lba_base;      // 本分区的起始 lba 地址
    uint32_t block_bitmap_lba;   // 块位图本身起始扇区地址
    uint32_t block_bitmap_sects; // 扇区位图本身占用的扇区数量
    uint32_t inode_bitmap_lba;   // i 结点位图起始扇区 lba 地址
    uint32_t inode_bitmap_sects; // i 结点位图占用的扇区数量
    uint32_t inode_table_lba;    // i 结点表起始扇区 lba 地址
    uint32_t inode_table_sects;  // i 结点表占用的扇区数量
    uint32_t data_start_lba;     // 数据区开始的第一个扇区号
    uint32_t root_inode_no;      // 根目录所在的 I 结点号
    uint32_t dir_entry_size;     // 目录项大小
    uint8_t pad[460];            // 加上 460 字节，凑够 512 字节 1 扇区大小

} __attribute__((packed));
#endif