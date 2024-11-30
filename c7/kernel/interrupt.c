#include "interrupt.h"
#include "global.h"
#include "io.h"
#include "stdint.h"
#include "print.h"

#define PIC_M_CTRL 0x20 // 主片的控制端口是 0x20
#define PIC_M_DATA 0x21 // 主片的数据端口是 0x21
#define PIC_S_CTRL 0xa0 // 从片的控制端口是 0xa0
#define PIC_S_DATA 0xa1 // 从片的数据端口是 0xa1
#define IDT_DESC_CNT 0x21

/* 中断门描述符结构体*/
struct gate_desc // 结构体中位置越偏下的成员，其地址越高。
{
  uint16_t func_offset_low_word; // 中断处理程序在目标代码段内偏移量
  uint16_t selector; // 中断处理程序目标代码段描述符选择子
  // 低32位
  uint8_t dcount; // 此项为双字计数字段，是门描述符中的第 4 字节
                  // 此项固定值，不用考虑
  uint8_t attribute;              // 属性
  uint16_t func_offset_high_word; // 中断处理程序在目标段内的偏移量高16位
};

static struct gate_desc idt[IDT_DESC_CNT]; // idt 是中断描述符表
                                           // 本质上就是个中断门描述符数组
/*初始化可编程中断控制器*/
static void pic_init(void) {

  /*初始化主片*/
  outb(PIC_M_CTRL,
       0x11); // ICW1: 通知8259A，现在开始初始化:边缘触发，级联8259 ，需要ICW4
  outb(PIC_M_DATA, 0x20); // ICW2: 设置中断向量号从0x20开始
                          // 也就是IR[0～7]0x20～0x27
  outb(PIC_M_DATA, 0x04); // ICW3: 设置从片连接到主片的IRQ2
  outb(PIC_M_DATA, 0x01); // ICW4: 8086模式，正常EOI

  /*初始化从片*/
  outb(PIC_S_CTRL,
       0x11); // ICW1: 通知8259A，现在开始初始化:边缘触发，级联8259 ，需要ICW4
  outb(PIC_S_DATA, 0x28); // ICW2: 设置中断向量号从0x28开始
                          // 也就是IR[8～15]0x28～0x2f
  outb(PIC_S_DATA, 0x02); // ICW3: 设置主片连接到从片的IR2引脚
  outb(PIC_S_DATA, 0x01); // ICW4: 8086模式，正常EOI

  /*打开主片上IR0,也就是目前只接受时钟产生的中断*/
  outb(PIC_M_DATA, 0xfe);
  outb(PIC_S_DATA, 0xff);
  put_str("  pic_init done\n");
}


// 静态函数声明，非必须
static void make_idt_desc(struct gate_desc *p_gate, uint8_t attr,
                          intr_handler function);

static struct gate_desc idt[IDT_DESC_CNT]; // idt是中断描述符表
                                            // 本质上就是个中断门描述符

extern intr_handler
    intr_entry_table[IDT_DESC_CNT]; // 声明中断处理程序数组
                                     // 定义在kernel.S中的中断函数入口数组

////////////////////

/* 创建中断门描述符 */
static void make_idt_desc(struct gate_desc *p_gdesc, uint8_t attr,
                          intr_handler function) {
  p_gdesc->func_offset_low_word = (uint32_t)function & 0x0000FFFF;
  p_gdesc->selector = SELECTOR_K_CODE; // 内核代码段选择子 。selector是段选择子
  p_gdesc->dcount = 0;
  p_gdesc->attribute = attr;
  p_gdesc->func_offset_high_word = ((uint32_t)function & 0xFFFF0000) >> 16;
}
/*初始化中断描述符表*/

static void idt_desc_init(void) {
  int i;
  for (i = 0; i < IDT_DESC_CNT; i++) { // 0x21个中断（32）
    make_idt_desc(&idt[i], IDT_DESC_ATTR_DPL0,
                  intr_entry_table[i]); // 初始化idt每一个中断门描述符
  }
  put_str("  idt_desc_init done\n");
}

/*完成有关中断的所有初始化工作*/
void idt_init(void) {
  put_str("idt_init start\n");
  idt_desc_init(); // 初始化中断描述符表
  pic_init();      // 初始化8259A中断控制器

  /*加载idt 75 */
  uint64_t idt_operand = ((sizeof(idt) - 1) | ((uint64_t)(uint32_t)idt << 16));
  asm volatile("lidt %0" : : "m"(idt_operand));
  put_str("idt_init done\n");
}
