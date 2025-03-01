#ifndef __DEVICE_IDE_H
#define __DEVICE_IDE_H
#include "bitmap.h"
#include "list.h"
#include "stdint.h"
#include "sync.h"
/* 分区结构 */
struct partition {
  uint32_t start_lba;         //
  uint32_t sec_cnt;           //
  struct disk *my_disk;       //
  struct list_elem part_tag;  //
  char name[8];               //
  struct superblock *sb;      //
  struct bitmap block_bitmap; //
  struct bitmap inode_bitmap; //
  struct list open_inodes;    //
};

/* 硬盘结构*/
struct disk {
  char name[8];                    // 本硬盘的名称
  struct ide_channel *my_channel;  // 本硬盘在ide通道中的
  uint8_t dev_no;                  // 本硬盘是主0,还是从1
  struct partition prim_parts[4];  // 主分区
  struct partition logic_parts[8]; // 逻辑分区
};

/* ata通道结构*/
struct ide_channel {
  char name[8];               // 本通道的名称
  uint16_t port_base;         // 数据起始端口地址
  uint8_t irq_no;             // 本通道所用的IRQ号中断
  struct lock lock;           // 通道锁
  bool expecting_intr;        // 期望接收到银盘中断
  struct semaphore disk_done; // 用于堵塞唤醒驱动方程
  struct disk devices[2];     // 两个硬盘 主0，从1
};

/* 硬盘数据结构初始化*/
void ide_init(void);

void intr_hd_handler(uint8_t irq_no);
/* 将buf中sec_cnt扇区数据写入硬盘 */
void ide_write(struct disk *hd, uint32_t lba, void *buf, uint32_t sec_cnt);
/* 从硬盘读取sec_cnt个扇区到buf */
void ide_read(struct disk *hd, uint32_t lba, void *buf, uint32_t sec_cnt);
#endif