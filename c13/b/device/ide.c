#include "ide.h"
#include "debug.h"
#include "global.h"
#include "stdio.h"
#include "io.h"
#include "timer.h"
/* 定义硬盘各寄存器端口号 */

#define reg_data(channel) (channel->port_base + 0)
#define reg_error(channel) (channel->port_base + 1)
#define reg_sect_cnt(channel) (channel->port_base + 2)
#define reg_lba_l(channel) (channel->port_base + 3)
#define reg_lba_m(channel) (channel->port_base + 4)
#define reg_lba_h(channel) (channel->port_base + 5)
#define reg_dev(channel) (channel->port_base + 6)
#define reg_status(channel) (channel->port_base + 7)
#define reg_cmd(channel) (reg_status(channel))
#define reg_alt_status(channel) (channel->port_base + 0x206)
#define reg_ctl(channel) reg_alt_status(channel)

/* reg_alt_status 寄存器的一些关键位 */
#define BIT_STAT_BSY 0x80      // 硬盘忙
#define BIT_ALT_STAT_DRDY 0x40 // 驱动器准备好
#define BIT_STAT_DRQ 0x8   // 数据传输准备好了

/* device 寄存器的一些关键位 */
#define BIT_DEV_MBS 0xa0 // 第 7 位和第 5 位固定为 1
#define BIT_DEV_LBA 0x40
#define BIT_DEV_DEV 0x10
/* 一些硬盘操作的指令  */
#define CMD_IDENTIFY                                                           \
  0xec // identify 指令#define BIT_STAT_BSY    0x80    // 硬盘忙
#define CMD_READ_SECTOR 0x20  // 读扇区指令
#define CMD_WRITE_SECTOR 0x30 // 写扇区指令

/* 定义可读写的最大扇区数，调试用的 */
#define max_lba ((80 * 1024 * 1024 / 512) - 1) // 只支持 80MB 硬盘
uint8_t channel_cnt;                           // 按硬盘数计算的通道数
struct ide_channel channels[2];                // 有两个 ide 通道

/* 选择读写的硬盘 */
static void ide_select_disk(struct disk *hd) {
  uint8_t reg_device = BIT_DEV_MBS | BIT_DEV_LBA;
  if (hd->dev_no == 1) { // 若是从盘就置DEV位为1
    reg_device |= BIT_DEV_DEV;
  }
  outb(reg_dev(hd->my_channel), reg_device);
}

/* 向硬盘控制器写入起始扇区地址及要读写的扇区数 */
static void select_sector(struct disk *hd, uint32_t lba, uint8_t sector_cnt) {
  ASSERT(lba <= max_lba);
  struct ide_channel *channel = hd->my_channel;

  /* 写入要读写的扇区数 */
  outb(reg_sect_cnt(channel), sector_cnt);
  // 如果sec_cnt 为0，则表示写入的扇区数为256

  /* 写入lba地址的7-0位 */
  outb(reg_lba_l(channel), lba);
  // lba 地址的低8位，不用单独取出低8位，直接赋值
  // outb函数中的汇编指令outb %b0，%w1 会只用al;
  outb(reg_lba_m(channel), lba >> 8);
  outb(reg_lba_h(channel), lba >> 16);

  /* 因为 lba 地址的第 24～27 位要存储在 device 寄存器的 0～3 位，
   * 无法单独写入这 4 位，所以在此处把 device 寄存器再重新写入一次*/
  outb(reg_dev(channel), BIT_DEV_MBS | BIT_DEV_LBA |
                             (hd->dev_no == 1 ? BIT_DEV_DEV : 0) | lba >> 24);
}

/* 向通道channel发命令cmd*/
static void cmd_out(struct ide_channel *channel, uint8_t cmd) {
  /*  只要向硬盘发出了命令便将此标记置为true
  硬盘中断处理程序需要根据它来判断 */
  channel->expecting_intr = true;
  outb(reg_cmd(channel), cmd);
}

/* 硬盘读入sec_cnt 个扇区的数据到buf */
static void read_from_sector(struct disk *hd, void *buf, uint8_t sec_cnt) {
  uint32_t size_in_byte;
  if (sec_cnt == 0) {
    /* 因为 sec_cnt 是 8 位变量，由主调函数将其赋值时，若为 256 则会将最高位的 1
     * 丢掉变为 0 */
    size_in_byte = 256 * 512;
  } else {
    size_in_byte = sec_cnt * 512;
  }
  insw(reg_data(hd->my_channel), buf, size_in_byte / 2);
}

/* 将buf中sec_cnt 扇区的数据写入硬盘 */
static void write2sector(struct disk *hd, void *buf, uint8_t sec_cnt) {
  uint32_t size_in_byte;
  if (sec_cnt == 0) {
    size_in_byte = 256 * 512;
  } else {
    size_in_byte = sec_cnt * 512;
  }
  outsw(reg_data(hd->my_channel), buf, size_in_byte / 2);
}

/* 等待30秒 */
static bool busy_wait(struct disk *hd) {
  struct ide_channel *channel = hd->my_channel;
  uint16_t time_limit = 30 * 1000; // 可以等待 30000 毫秒
  while (time_limit -= 10 >= 0) {
    if (!(inb(reg_status(channel)) & BIT_STAT_BSY)) {
      return (inb(reg_status(channel)) & BIT_STAT_DRQ);

    } else {
      mtime_sleep(10); // 睡眠 10 毫秒
    }
  }
  return false;
}

/* 硬盘数据结构初始化*/
void ide_init(void) {
  printf("ide_init start\n");
  uint8_t hd_cnt = *((uint8_t *)(0x475)); // 获取银盘数量
  ASSERT(hd_cnt > 0);
  channel_cnt = DIV_ROUND_UP(hd_cnt, 2);
  struct ide_channel *channel;
  uint8_t channel_no = 0;
  /* 处理每一个通道上的硬盘*/
  while (channel_no < channel_cnt) {
    channel = &channels[channel_no];
    sprintf(channel->name, "ide%d", channel_no);

    /* 为了每个ide通道初始化端口基址及中断向量*/
    switch (channel_no) {
    case 0:
      channel->port_base = 0x1f0;
      // ide0通道的起始端口号是0x1f0
      channel->irq_no = 14 + 0x20;
      // 从片8259a上倒数第二的中断引脚
      //  硬盘，也就是ide0通道的中断向量号
      break;
    case 1:
      channel->port_base = 0x170;
      // ide1通道的起始端口号是0x170
      channel->irq_no = 15 + 0x20;
      // 从片8259a上倒数第一的中断引脚
      // 我们用来响应ide1通道上的硬盘中断
      break;
    }
    channel->expecting_intr = false; // 为向硬盘写入指令时不期待硬盘的中断
    lock_init(&channel->lock);
    /* 初始化为 0，目的是向硬盘控制器请求数据后，
 硬盘驱动 sema_down 此信号量会阻塞线程，
 直到硬盘完成后通过发中断，
 由中断处理程序将此信号量 sema_up，唤醒线程 */
    sema_init(&channel->disk_done, 0);
    channel_no++; // 下一个channel
  }

  printf("ide_init end\n");
}
