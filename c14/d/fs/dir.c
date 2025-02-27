#include "dir.h"
#include "bitmap.h"
#include "debug.h"
#include "file.h"
#include "inode.h"
#include "stdio-kernel.h"
#include "thread.h"
#include "string.h"
struct dir root_dir; // 根目录

struct partition *cur_part; // 默认情况下操作的是哪个分区

/* 打开根目录 */
void open_root_dir(struct partition *part) {

  root_dir.inode = inode_open(part, part->sb->root_inode_no);
  root_dir.dir_pos = 0;
}
/* 在分区part上打开i结点为indoe_no 的 目录并返回目录指针 */
struct dir *dir_open(struct partition *part, uint32_t inode_no) {
  struct dir *pdir = (struct dir *)sys_malloc(sizeof(struct dir));
  pdir->inode = inode_open(part, inode_no);
  pdir->dir_pos = 0;
  return pdir;
}

/*  在part分区内的pdir目录内寻找名为name 的文件或目录
 *   找到后返回true 并将其目录项存入dir_e，否则返回false */
bool search_dir_entry(struct partition *part, struct dir *pdir,
                      const char *name, struct dir_entry *dir_e) {
  uint32_t block_cnt = 140; // 12个直接块 + 256个一级索引块 + 128个一级间接块
  /* 12个直接大小 + 128 个间接块,一共560个字节 */
  uint32_t *all_blocks = (uint32_t *)sys_malloc(512 + 48);
  if (all_blocks == NULL) {
    printk("search_dir_entry: sys_malloc for all_blocks failed\n");
    return false;
  }
  uint32_t block_idx = 0;
  while (block_idx < 12) {
    all_blocks[block_idx] = pdir->inode->i_sectors[block_idx];
    block_idx++;
  }
  block_idx = 0;
  if (pdir->inode->i_sectors[12] != 0) { // 如果含有一级间接块
    ide_read(part->my_disk, pdir->inode->i_sectors[12], all_blocks + 12, 1);
  }
  /* 至此，all_blocks 存储的是该文件或目录的所有扇区地址 */
  /* 写目录项的时候已保证目录项不跨扇区，
   * 这样读目录项时容易处理，只申请容纳 1 个扇区的内存 */
  uint8_t *buf = (uint8_t *)sys_malloc(SECTOR_SIZE);
  struct dir_entry *p_de = (struct dir_entry *)buf;
  // p_de是指向目录项的指针，值为buf起始地址
  uint32_t dir_entry_size = part->sb->dir_entry_size;
  uint32_t dir_entry_cnt = SECTOR_SIZE / dir_entry_size;
  // 1扇区可以容纳目录项数目

  /*  开始在所在块中查找目录项 */
  while (block_idx < block_cnt) {
    {
      /* 块地址为0 表述这个块没有数据，继续在其他块中找 */
      if (all_blocks[block_idx] == 0) {
        block_idx++;
        continue;
      }
      ide_read(part->my_disk, all_blocks[block_idx], buf, 1);
      uint32_t dir_entry_idx = 0;
      /* 遍历扇区中所有目录项 */
      while (dir_entry_idx < dir_entry_cnt) {
        /* 如果找到了就直接复制整个目录项  */
        if (!strcmp(p_de->filename, name)) {
          memcpy(dir_e, p_de, dir_entry_size);
          sys_free(buf);
          sys_free(all_blocks);
          return true;
        }
        dir_entry_idx++;
        p_de++;
      }
      block_idx++;
      p_de++;
    }
    block_idx++;
    p_de = (struct dir_entry *)buf;
    // 此时p_de已经指向扇区最后一共完整目录项
    //  需要恢复p_de指向为buf
    memset(buf, 0, SECTOR_SIZE); // 将buf清0 下次再用
  }
  sys_free(buf);
  sys_free(all_blocks);
  return false;
}

/* 关闭目录 */
void dir_close(struct dir *dir) {
  /************* 根目录不能关闭 ***************
   *1 根目录自打开后就不应该关闭，否则还需要再次 open_root_dir();
   *2 root_dir 所在的内存是低端 1MB 之内，并非在堆中，free 会出问题 */
  if (dir == &root_dir) {
    /* 直接返回*/
    return;
  }
  inode_close(dir->inode);
  sys_free(dir);
}

/* 在内存中初始化目录项p_de*/
void create_dir_entry(char *filename, uint32_t inode_no, uint8_t file_type,
                      struct dir_entry *p_de) {
  ASSERT(strlen(filename) <= MAX_FILE_NAME_LEN);
  /* 初始化目录项 */
  memcpy(p_de->filename, filename, strlen(filename));
  p_de->i_no = inode_no;
  p_de->f_type = file_type;
}

/* 将目录项 p_de 写入父目录 parent_dir 中，io_buf 由主调函数提供 */

bool sync_dir_entry(struct dir *parent_dir, struct dir_entry *p_de,
                    void *io_buf) {
  struct inode *dir_inode = parent_dir->inode;
  uint32_t dir_size = dir_inode->i_size;
  uint32_t dir_entry_size = cur_part->sb->dir_entry_size;

  ASSERT(dir_size % dir_entry_size == 0);
  // dir_size 应该是dir_entry_size的整数被
  uint32_t dir_entry_per_sec = (512 / dir_entry_size);
  // 扇区最大的目录项数目
  int32_t block_lba = -1;

  /* 将该目录的所有扇区地址
  （12 个直接块 + 128 个间接块 ）存在all_blocks */
  uint8_t block_idx = 0;
  uint32_t all_blocks[140] = {0}; // all_blocks 保存目录所有的块
  /* 将12个直接块存入all_blocks */
  while (block_idx < 12) {
    all_blocks[block_idx] = dir_inode->i_sectors[block_idx];
    block_idx++;
  }
  struct dir_entry *dir_e = (struct dir_entry *)io_buf;
  // dir_e用来在io_buf 中遍历目录项
  int32_t block_bitmap_idx = -1;

  /* 开始遍历所有块以寻找目录项空位，若已有扇区中没有空闲位，
   * 在不超过文件大小的情况下申请新扇区来存储新目录项 */
  block_idx = 0;
  while (block_idx < 140) {
    // 文件（包括目录）最大支持 12 个直接块+128 个间接块＝140 个块
    block_bitmap_idx = -1;
    if (all_blocks[block_idx] == 0) { // 在三种情况下分配块
      block_lba = block_bitmap_alloc(cur_part);
      if (block_lba == -1) {
        printk("alloc block bitmap for sync_dir_entry failed\n");
        return false;
      }
      /* 每分配一个块就同步一次 block_bitmap */
      block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
      ASSERT(block_bitmap_idx != -1);
      bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
      block_bitmap_idx = -1;
      if (block_idx < 12) { // 直接块
        dir_inode->i_sectors[block_idx] = all_blocks[block_idx] = block_lba;
      } else if (block_idx == 12) {
        // 若是尚未分配一级间接块表（block_idx 等于 12 表示第 0 个间接块地址为
        // 0）
        dir_inode->i_sectors[12] = block_lba;
        // 将上面分配的块作为一级间接块表地址
        block_lba = -1;
        block_lba = block_bitmap_alloc(cur_part);
        // 再分配一个块作为第 0 个间接块
        if (block_lba == -1) {
          block_bitmap_idx =
              dir_inode->i_sectors[12] - cur_part->sb->data_start_lba;
          bitmap_set(&cur_part->block_bitmap, block_bitmap_idx, 0);
          dir_inode->i_sectors[12] = 0;
          printk("alloc block bitmap for sync_dir_entry failed\n");
          return false;
        }
        /* 每分配一个块就同步一次 block_bitmap */
        block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
        ASSERT(block_bitmap_idx != -1);
        bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);

        all_blocks[12] = block_lba;
        /* 把新分配的第0个间接块地址写入一级间接块表 */
        ide_write(cur_part->my_disk, dir_inode->i_sectors[12], all_blocks + 12,
                  1);
      } else { // 若是间接块未分配
        all_blocks[block_idx] = block_lba;
        /* 把新分配的第(block_idx-12)个间接块地址写入一级间接块表 */
        ide_write(cur_part->my_disk, dir_inode->i_sectors[12], all_blocks + 12,
                  1);
      }
      /* 再将新目录项 p_de 写入新分配的间接块 */
      memset(io_buf, 0, 512);
      memcpy(io_buf, p_de, dir_entry_size);
      ide_write(cur_part->my_disk, all_blocks[block_idx], io_buf, 1);
      dir_inode->i_size += dir_entry_size;
      return true;
    }
    /* 若第 block_idx 块已存在，将其读进内存，然后在该块中查找空目录项 */
    ide_read(cur_part->my_disk, all_blocks[block_idx], io_buf, 1);
    /* 在扇区内查找空目录项 */
    uint8_t dir_entry_idx = 0;
    while (dir_entry_idx < dir_entry_per_sec) {
      if ((dir_e + dir_entry_idx)->f_type == FT_UNKNOWN) {
        // FT_UNKNOWN 为 0，无论是初始化，或是删除文件后，
        // 都会将 f_type 置为 FT_UNKNOWN
        memcpy(dir_e + dir_entry_idx, p_de, dir_entry_size);
        ide_write(cur_part->my_disk, all_blocks[block_idx], io_buf, 1);
        dir_inode->i_size += dir_entry_size;
        return true;
      }
      dir_entry_idx++;
    }
    block_idx++;
  }
  printk("directory is full!\n");
  return false;
}
