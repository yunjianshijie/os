#include "fs.h"
#include "../kernel/memory.h"
#include "bitmap.h"
#include "debug.h"
#include "dir.h"
#include "file.h"
#include "ide.h"
#include "inode.h"
#include "list.h"
#include "stdio-kernel.h"
#include "string.h"
#include "super_block.h"
extern struct file file_table[MAX_FILE_OPEN];
extern uint8_t channel_cnt;            // 记录通道数
extern struct ide_channel channels[2]; // 有两个ide通道
extern struct list partition_list;     // 分区链表
extern struct partition *cur_part;     // 默认情况下操作的是哪个分区
extern struct dir root_dir;            // 根目录
/*  格式化分区，也就是初始化分区的元信息，创建文件系统 */
static void partition_format(struct partition *part) {
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

  /* 1 先将位图最后一字节到其所在的扇区的结束全置为 1，
  即超出实际块数的部分直接置为已占用*/
  memset(&buf[block_bitmap_last_byte], 0xff, last_size);
  /* 2 再将上一步中覆盖的最后一字节内的有效位重新置 0 */
  uint8_t bit_idx = 0;
  while (bit_idx <= block_bitmap_last_bit) {
    buf[block_bitmap_last_byte] &= ~(1 << bit_idx++);
  }
  ide_write(hd, sb.block_bitmap_lba, buf, sb.block_bitmap_sects);
  /***************************************
   * 3 将 inode 位图初始化并写入 sb.inode_bitmap_lba *
   ***************************************/
  /* 先清空缓冲区*/
  memset(buf, 0, buf_size);
  buf[0] |= 0x01; // 第0个indoe分给更目录

  /* 由于 inode_table 中共 4096 个 inode，
   * 位图 inode_bitmap 正好占用 1 扇区，
   * 即 inode_bitmap_sects 等于 1，
   * 所以位图中的位全都代表 inode_table 中的 inode，
   * 无需再像 block_bitmap 那样单独处理最后一扇区的剩余部分，
   * inode_bitmap 所在的扇区中没有多余的无效位 */
  ide_write(hd, sb.inode_bitmap_lba, buf, sb.inode_bitmap_sects);

  /***************************************
   * 4 将 inode 数组初始化并写入 sb.inode_table_lba *
   ***************************************/
  /* 准备写 inode_table 中的第 0 项,即根目录所在的 inode */

  memset(buf, 0, buf_size); // 先清空缓冲区 buf
  struct inode *i = (struct inode *)buf;
  i->i_size = sb.dir_entry_size * 2; // .和..
  i->i_no = 0;                       // 根目录占inode数组中第0个inode
  i->i_sectors[0] = sb.data_start_lba;
  // 由于上面的 memset，i_sectors 数组的其他元素都初始化为 0

  ide_write(hd, sb.inode_table_lba, buf, sb.inode_table_sects);

  /***************************************
   * 5 将根目录写入 sb.data_start_lba
   ***************************************/

  /* 写入根目录的,和..*/
  memset(buf, 0, buf_size);
  struct dir_entry *p_de = (struct dir_entry *)buf;

  /* 初始化当前目录 ". "*/
  memcpy(p_de->filename, ".", 1);
  p_de->i_no = 0;
  p_de->f_type = FT_DIRECTORY;
  p_de++;

  /* 初始化当前目录的父目录 ".."*/
  memcpy(p_de->filename, "..", 2);
  p_de->i_no = 0; // 父目录是根目录，i_no 也为 0
  p_de->f_type = FT_DIRECTORY;

  /* sb.data_start_lba 已经分配给了根目录，里面是根目录的目录项 */
  ide_write(hd, sb.data_start_lba, buf, 1);

  printk(" root_dir_lba:0x%x\n", sb.data_start_lba);
  printk("%s format done\n", part->name);
  sys_free(buf);
}
/* 在磁盘上搜索文件系统，若没有则格式化分区创建文件系统 */
void filesys_init(void) {
  uint8_t channel_no = 0, dev_no, part_idx = 0;
  /* sb_buf 用来存储从硬盘上读入的超级块 */
  struct super_block *sb_buf = (struct super_block *)sys_malloc(SECTOR_SIZE);
  if (sb_buf == NULL) {
    PANIC("alloc memory failed!");
  }
  printk("searching filesystem .......");
  while (channel_no < channel_cnt) {
    dev_no = 0;
    while (dev_no < 2) {
      if (dev_no == 0) { // 跨过裸盘hd60M.img
        dev_no++;
        continue;
      }
      struct disk *hd = &channels[channel_no].devices[dev_no];
      struct partition *part = hd->prim_parts;
      while (part_idx < 12) // 4个主分区+8个逻辑
      {
        if (part_idx == 4) { // 开始处理逻辑分区
          part = hd->logic_parts;
        }
        /* channels 数组是全局变量，默认值为 0，disk 属于其嵌套结构，
         * partition 又为 disk 的嵌套结构，因此 partition 中的成员默认也为 0。
         * 若 partition 未初始化，则 partition 中的成员仍为 0。
         * 下面处理存在的分区 */
        if (part->sec_cnt != 0) { // 如果分区存在
          memset(sb_buf, 0, SECTOR_SIZE);
          /* 读出分区的超级块，根据魔数是否正确来判断是否存在文件系统 */
          ide_read(hd, part->start_lba + 1, sb_buf, 1);
          /* 只支持自己的文件系统，若磁盘上已经有文件系统就不再格式化了 */
          if (sb_buf->magic == 0x19590318) {
            printk("%s has filesystem\n", part->name);
          } else {
            printk("formatting %s`s partition %s......\n", hd->name,
                   part->name);
            partition_format(part);
          }
        }
        part_idx++;
        part++; // 下一个分区
      }
      dev_no++; // 下一磁盘
    }
    channel_no++; // 下一通道
  }
  sys_free(sb_buf);
  /* 确定默认操作的分区 */
  char default_part[8] = "sdb1";
  /* 挂载分区 */
  list_traversal(&partition_list, mount_partition, (int)default_part);
  /* 将当前分区的根目录打开 */
  open_root_dir(cur_part);

  /* 初始化文件表 */
  uint32_t fd_idx = 0;
  while (fd_idx < MAX_FILE_OPEN) {
    file_table[fd_idx++].fd_inode = NULL;
  }
}

/* 在分区链表中找到名为 part_name 的分区，并将其指针赋值给 cur_part */
static bool mount_partition(struct list_elem *pelem, int arg) {
  char *part_name = (char *)arg;
  struct partition *part = elem2entry(struct partition, part_tag, pelem);
  if (!strcmp(part->name, part_name)) {
    cur_part = part;
    struct disk *hd = cur_part->my_disk;

    /* sb_buf 用来存储从硬盘上读入的超级块 */
    struct super_block *sb_buf = (struct super_block *)sys_malloc(SECTOR_SIZE);
    /* 在内存中创建分区cur_part的超级块 */
    cur_part->sb = (struct super_block *)sys_malloc(sizeof(struct super_block));
    if (cur_part->sb == NULL) {
      PANIC("alloc memory failed!");
    }
    /* 读入超级块*/
    memset(sb_buf, 0, SECTOR_SIZE);
    ide_read(hd, cur_part->start_lba + 1, sb_buf, 1);
    /* 把 sb_buf 中超级块的信息复制到分区的超级块 sb 中 */
    memcpy(cur_part->sb, sb_buf, sizeof(struct super_block));
    /********** 将硬盘上的块位图读入到内存 **************/
    cur_part->block_bitmap.bits =
        (uint8_t *)sys_malloc(sb_buf->block_bitmap_sects * SECTOR_SIZE);
    if (cur_part->block_bitmap.bits == NULL) {
      PANIC("alloc memory failed!");
    }
    cur_part->block_bitmap.btmp_bytes_len =
        sb_buf->block_bitmap_sects * SECTOR_SIZE;
    /* 从硬盘上读入块位图到分区的 block_bitmap.bits */
    ide_read(hd, sb_buf->block_bitmap_lba, cur_part->block_bitmap.bits,
             sb_buf->block_bitmap_sects);
    /*************************************************************/

    /********** 将硬盘上的 inode 位图读入到内存 ************/
    cur_part->inode_bitmap.bits =
        (uint8_t *)sys_malloc(sb_buf->inode_bitmap_sects * SECTOR_SIZE);
    if (cur_part->inode_bitmap.bits == NULL) {
      PANIC("alloc memory failed!");
    }
    cur_part->inode_bitmap.btmp_bytes_len =
        sb_buf->inode_bitmap_sects * SECTOR_SIZE;
    /* 从硬盘上读入 inode 位图到分区的 inode_bitmap.bits */
    ide_read(hd, sb_buf->inode_bitmap_lba, cur_part->inode_bitmap.bits,
             sb_buf->inode_bitmap_sects);
    /*************************************************************/
    list_init(&cur_part->open_inodes);
    printk("mount %s done!\n", part->name);
    /* 此处返回 true 是为了迎合主调函数 list_traversal 的实现，
     * 与函数本身功能无关。
     * 只有返回 true 时 list_traversal 才会停止遍历，
     * 减少了后面元素无意义的遍历 */
    return true;
  }
  return false; // 使 list_traversal 继续遍
}

/* 将最上层路径名称解析出来 */
static char *path_parse(char *pathname, char *name_store) {
  if (pathname[0] == '/') { // 根目录不需要单独解析
    /* 路径中出现1个或多个连续的字符 '/' ， 将这些‘/’跳过，如果 "///a/b" */
    while (*(++pathname) == '/')
      ;
  }
  /* 开始一般的路径解析*/
  while (*pathname != '/' && *pathname != 0) {
    *name_store++ = *pathname++;
  }
  if (pathname[0] == 0) {
    // 若路径字符串为空，则返回NULL
    return NULL;
  }
  return pathname;
}

/* 返回路径深度，比如/a/b/c，深度为 3 */
int32_t path_depth_cnt(char *pathname) {
  ASSERT(pathname != NULL);
  char *p = pathname;
  char name[MAX_FILE_NAME_LEN]; // 用于 path_parse 的参数做路径解析
  uint32_t depth = 0;
  /* 解析路径，从中拆分出各级名称 */
  p = path_parse(p, name);
  while (name[0]) {
    depth++;
    memset(name, 0, MAX_FILE_NAME_LEN);
    if (p) { // 如果 p 不等于 NULL，继续分析路径
      p = path_parse(p, name);
    }
  }
  return depth;
}

/* 搜索文件 pathname，若找到则返回其 inode 号，否则返回-1 */
static int search_file(const char *pathname,
                       struct path_search_record *searched_record) {
  /* 如果待查找的是根目录，为避免下面无用的查找，
   直接返回已知根目录信息 */
  if (!strcmp(pathname, "/") || !strcmp(pathname, "/.") ||
      !strcmp(pathname, "/..")) {
    searched_record->parent_dir = &root_dir;
    searched_record->file_type = FT_DIRECTORY;
    searched_record->searched_path[0] = 0; // 搜索路径置空
    return 0;
  }

  uint32_t path_len = strlen(pathname);
  /* 保证 pathname 至少是这样的路径/x，且小于最大长度 */
  ASSERT(pathname[0] == '/' && path_len > 1 && path_len < MAX_PATH_LEN);
  char *sub_path = (char *)pathname;
  struct dir *parent_dir = &root_dir;
  struct dir_entry dir_e;
  /* 记录路径解析出来的各级名称，如路径"/a/b/c"，
   * 数组 name 每次的值分别是"a","b","c" */
  char name[MAX_FILE_NAME_LEN] = {0};

  searched_record->parent_dir = parent_dir;
  searched_record->file_type = FT_UNKNOWN;
  uint32_t parent_inode_no = 0; // 父目录的inode编号
  sub_path = path_parse(sub_path, name);
  while (name[0]) { // 若第一个字符就是结束符，结束循环
    /* 记录查找过的路径，但不能超过 searched_path 的长度 512 字节 */
    ASSERT(strlen(searched_record->searched_path) < 512);

    /* 记录已存在的父目录 */
    strcat(searched_record->searched_path, "/");
    strcat(searched_record->searched_path, name);

    /* 在所给的目录中查找文件 */
    if (search_dir_entry(cur_part, parent_dir, name, &dir_e)) {
      memset(name, 0, MAX_FILE_NAME_LEN);
      /* 若 sub_path 不等于 NULL，也就是未结束时继续拆分路径 */
      if (sub_path) {
        sub_path = path_parse(sub_path, name);
      }
      if (FT_DIRECTORY == dir_e.f_type) { // 如果被打开的是目录
        parent_inode_no = parent_dir->inode->i_no;
        dir_close(parent_dir);
        parent_dir = dir_open(cur_part, dir_e.i_no); // 更新父目录
        searched_record->parent_dir = parent_dir;
        continue;
      } else if (FT_REGULAR == dir_e.f_type) { // 如果被打开的是普通文件
        searched_record->file_type = FT_REGULAR;
        return dir_e.i_no;
      }
    } else {
      /* 没有找到，则返回 -1 */
      /* 找不到目录项时，要留着 parent_dir 不要关闭，
       * 若是创建新文件的话需要在 parent_dir 中创建 */
      return -1;
    }
  }
  /* 执行到此，必然是遍历了完整路径， 并且查找的文件或目录只有同名目录存在 */
  dir_close(searched_record->parent_dir);
  /* 保存被查找目录的直接父目录 */
  searched_record->parent_dir = dir_open(cur_part, parent_inode_no);
  searched_record->file_type = FT_DIRECTORY;
  return dir_e.i_no;
}

/* 打开或创建文件成功后，返回文件描述符，否则返回-1 */
int32_t sys_open(const char *pathname, uint8_t flags) {
  /* 对目录要用 dir_open，这里只有 open 文件 */
  if (pathname[strlen(pathname) - 1] == '/') {
    printk("can`t open a directory %s\n", pathname);
    return -1;
  }
  ASSERT(flags <= 7);
  int32_t fd = -1; // 默认为找不到

  struct path_search_record searched_record;
  memset(&searched_record, 0, sizeof(struct path_search_record));

  /* 记录目录深度，帮助判断中间某个目录不存在的情况 */
  uint32_t pathname_depth = path_depth_cnt((char *)pathname);

  /* 先检查文件是否存在 */
  int inode_no = search_file(pathname, &searched_record);
  bool found = inode_no != -1 ? true : false;
  if (searched_record.file_type == FT_DIRECTORY) {
    printk("can`t open a direcotry with open(), use opendir() to instead\n");
    dir_close(searched_record.parent_dir);
    return -1;
  }
  uint32_t path_searched_depth = path_depth_cnt(searched_record.searched_path);
  /* 先判断是否把 pathname 的各层目录都访问到了，
   即是否在某个中间目录就失败了 */
  if (pathname_depth != path_searched_depth) {
    // 说明并没有访问到全部的路径，某个中间目录是不存在的
    printk("cannot access %s: Not a directory, subpath %s is’t exist\n",
           pathname, searched_record.searched_path);
    dir_close(searched_record.parent_dir);
    return -1;
  }
  /* 若是在最后一个路径上没找到，并且并不是要创建文件，直接返回-1 */
  if (!found && !(flags & O_CREAT)) {
    printk("in path %s, file %s is`t exist\n", searched_record.searched_path,
           (strrchr(searched_record.searched_path, '/') + 1));
    dir_close(searched_record.parent_dir);
    return -1;
  } else if (found && flags & O_CREAT) { // 若要创建的文件已存在
    printk("%s has already exist!\n", pathname);
    dir_close(searched_record.parent_dir);
    return -1;
  }
  switch (flags & O_CREAT) {
  case O_CREAT:
    printk("creating file\n");
    fd = file_create(searched_record.parent_dir, (strrchr(pathname, '/') + 1),
                     flags);
    dir_close(searched_record.parent_dir);
    // 其余为打开文件;
  }
  /* 此 fd 是指任务 pcb->fd_table 数组中的元素下标，
   * 并不是指全局 file_table 中的下标 */
  return fd;
}
