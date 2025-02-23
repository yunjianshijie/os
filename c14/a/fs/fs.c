#include "fs.h"
#include "../kernel/memory.h"
#include "bitmap.h"
#include "dir.h"
#include "ide.h"
#include "inode.h"
#include "string.h"
#include "super_block.h"
/*  格式化分区，也就是初始化分区的元信息，创建文件系统 */
static void partition_format(struct disk *hd, struct partition *part) {
  /* blocks_bitmap_init（为方便实现，一个块大小是一扇区）*/
  uint32_t boot_sector_sects = 1;
  uint32_t super_block_sects = 1;
  uint32_t inode_bitmap_sects =
      DIV_ROUND_UP(MAX_FILES_PER_RART, BITS_PER_SECTOR);
  // I结点位图占用的扇区数，最多支持4096个文件
  uint32_t inode_table_sects =
      DIV_ROUND_UP((sizeof(struct inode) * MAX_FILES_PER_RART), SECTOR_SIZE);
  uint32_t used_sects = boot_sector_sects + super_block_sects +
                        inode_bitmap_sects + inode_table_sects;
  uint32_t free_sects = part->sec_cnt - used_sects;

  /********************** 简单处理块位图占据的扇区数 ******************** */
  uint32_t block_bitmap_sects;
  block_bitmap_sects = DIV_ROUND_UP(free_sects, BITS_PER_SECTOR);
  /* block_bitmap_bit_len 是位图中位的长度，也是可用块的数量 */
  uint32_t block_bitmap_bit_len = free_sects - block_bitmap_sects;
  block_bitmap_sects = DIV_ROUND_UP(block_bitmap_bit_len, BITS_PER_SECTOR);
  /*********************************************************/
  /* 超级块初始化 */
  struct super_block sb;
  sb.magic = 0x19570716;
  sb.sec_cnt = part->sec_cnt;
  sb.inode_cnt = MAX_FILES_PER_RART;
  sb.part_lba_base = part->start_lba;
  sb.block_bitmap_lba = sb.part_lba_base + 2;
  // 第0块是引导扇区，第1块是超级块
  sb.block_bitmap_sects = block_bitmap_sects;

  sb.inode_bitmap_lba = sb.block_bitmap_lba + sb.block_bitmap_sects;
  sb.inode_bitmap_sects = inode_bitmap_sects;

  sb.inode_table_lba = sb.inode_bitmap_lba + sb.inode_bitmap_sects;
  sb.inode_table_sects = inode_table_sects;

  sb.data_start_lba = sb.inode_table_lba + sb.inode_table_sects;
  sb.root_inode_no = 0;
  sb.dir_entry_size = sizeof(struct dir_entry);

  printk("%s info:\n", part->name);
  printk(" magic:0x%x\n part_lba_base:0x%x\n all_sectors : 0x %x\n inode_cnt : "
         "0x % x\nblock_bitmap_lba : 0x %x\n block_bitmap_sectors : 0x % x\n "
         "inode_bitmap_lba : 0x %x\n inode_bitmap_sectors : 0x %x "
         "\ninode_table_lba : 0x %x\n inode_table_sectors : 0x %x "
         "\ndata_start_lba : 0x %x\n ",
         sb.magic, sb.part_lba_base, sb.sec_cnt, sb.inode_cnt,
         sb.block_bitmap_lba, sb.block_bitmap_sects, sb.inode_bitmap_lba,
         sb.inode_bitmap_sects, sb.inode_table_lba, sb.inode_table_sects,
         sb.data_start_lba);
  struct disk *hd = part->my_disk;
  /*****************************
   * 1 将超级块写入本分区的1扇区
   *****************************/
  ide_write(hd, part->start_lba + 1, &sb, 1);
  printk(" super_block_lba:0x%x\n", part->start_lba + 1);

  /* 找出数据量最大的元信息，用其尺寸做存储缓冲区*/
  uint32_t buf_size =
      (sb.block_bitmap_sects >= sb.inode_bitmap_sects ? sb.block_bitmap_sects
                                                      : sb.inode_bitmap_sects);
  buf_size =
      (buf_size >= sb.inode_table_sects ? buf_size : sb.inode_table_sects) *
      SECTOR_SIZE;
  uint8_t *buf = (uint8_t *)sys_malloc(buf_size);
  // 申请的内存管理系统清0后返回

  /**************************************
   * 2 将块位图初始化并写入 sb.block_bitmap_lba *
   *************************************/
  /* 初始化位图初始化并写出sb.b_b_l*/
  buf[0] |= 0x01; // 将第0位设置为1，表示第0块被占用
  uint32_t block_bitmap_last_byte = block_bitmap_bit_len / 8;
  uint8_t block_bitmap_last_bit = block_bitmap_bit_len % 8;
  uint32_t last_size = SECTOR_SIZE - (block_bitmap_last_byte % SECTOR_SIZE);
  // last_size 是位图所在最后一个扇区中，不足一扇区的其余部分

  /* 1 先将位图*/
}