#include "interrupt.h"
#include "global.h"
#include "io.h"
#include "print.h"
#include "stdint.h"
#include "debug.h"
#define PIC_M_CTRL 0x20 // 主片的控制端口是 0x20
#define PIC_M_DATA 0x21 // 主片的数据端口是 0x21
#define PIC_S_CTRL 0xa0 // 从片的控制端口是 0xa0
#define PIC_S_DATA 0xa1 // 从片的数据端口是 0xa1

#define IDT_DESC_CNT 0x21 // 支持的中断描述符个数33

#define EFLAGS_IF 0x00000200 // IF 标志位，代表中断是否被禁止

#define GET_EFLAGS(EFLAG_VAR) asm volatile("pushfl\n" "popl %0" : "=g"(EFLAG_VAR)) //
//用REFLAG_VAR来保存EFLAGS寄存器的值

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

char *intr_name[IDT_DESC_CNT]; // 用来保存异常的名字
intr_handler idt_table
    [IDT_DESC_CNT]; // 用来保存中断处理程序,在kernel.S中定义的intrXXentry
// 只是处理程序的入口，最终调用的是ide_table中所指向的函数

static struct gate_desc idt[IDT_DESC_CNT]; // idt 是中断描述符表
                                           // 本质上就是个中断门描述符数组

extern intr_handler intr_entry_table[IDT_DESC_CNT];
// 声明引用定义在 kernel.S中的中断处理`函数`入口数组

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

// general 处理通用中断，通过用vec_nr（中断向量号）来确定处理的具体中断类型
static void general_intr_handler(uint8_t vec_nr) {
  // 如果中断向量号是0x27 在 IRQ7和IRQ15上，会产生为中断，不需要处理（spurious
  // interrupt） 0x2f是从片8259A的最后一个IRQ引脚，保留不用
  if (vec_nr == 0x27 || vec_nr == 0x2f) {
    return;
  }
  // 其他中断号
  put_str("int vector: 0x");
  put_int(vec_nr);
  put_char('\n');
}

/* 完成一般中断处理函数注册以及异常名称注册*/
static void exception_init(void) {
  int i;
  for (i = 0; i < IDT_DESC_CNT; i++) {
    /* idt_table 数组中的函数是在进入中断后根据中断向量号调用的
     *   见kernel/kernel.S的call [idt_table + %1*4]*/
    idt_table[i] = general_intr_handler;
    // 先默认为general_intr_handler
    // 以后会由register_handler来注册具体处理函数
    intr_name[i] = "unknown"; // 先统一赋值为unknown
  }
  intr_name[0] = "#DE Divide Error";
  intr_name[1] = "#DB Debug Exception";
  intr_name[2] = "NMI Interrupt";
  intr_name[3] = "#BP Breakpoint Exception";
  intr_name[4] = "#OF Overflow Exception";
  intr_name[5] = "#BR BOUND Range Exceeded Exception";
  intr_name[6] = "#UD Invalid Opcode Exception";
  intr_name[7] = "#NM Device Not Available Exception";
  intr_name[8] = "#DF Double Fault Exception";
  intr_name[9] = "Coprocessor Segment Overrun";
  intr_name[10] = "#TS Invalid TSS Exception";
  intr_name[11] = "#NP Segment Not Present";
  intr_name[12] = "#SS Stack Fault Exception";
  intr_name[13] = "#GP General Protection Exception";
  intr_name[14] = "#PF Page-Fault Exception";
  // intr_name[15] 第 15 项是 intel 保留项,未使用
  intr_name[16] = "#MF x87 FPU Floating-Point Error";
  intr_name[17] = "#AC Alignment Check Exception";
  intr_name[18] = "#MC Machine-Check Exception";
  intr_name[19] = "#XF SIMD Floating-Point Exception";
}
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

/*开中断并返回开中断前的状态*/
enum intr_status intr_enable() {
  enum intr_status old_status;
  if (INTR_ON == intr_get_status()) {
    old_status = INTR_ON;
  } else {
    old_status = INTR_OFF;
    asm volatile("sti"); // sti指令会设置处理器的中断标志，打开中断标志
  }
  return old_status;
}

/*关中断并返回关中断前的状态*/
enum intr_status intr_disable() {
  enum intr_status old_status;
  if (INTR_ON == intr_get_status()) {
    old_status = INTR_ON;
    asm volatile("cli"); // cli指令会清除处理器的中断标志，关闭中断标志
  } else {
    old_status = INTR_OFF;
  }
  return old_status;
}

/* 将中断状态设置为status*/
enum intr_status intr_set_status(enum intr_status status) {
    // enum intr_status old_status;
    // if(INTR_ON == intr_get_status()){
    //     old_status = INTR_ON;
    //     if(INTR_OFF == status){
    //       asm volatile("cli" : : : "memory"); // 关中断，cli 指令将 IF 位置 0 
    //       //  menory 是一个内存屏障，防止编译器优化
    //     }
    //   }
    return status & INTR_ON ? intr_disable():intr_enable();
    //妙啊，但是&INTR_ON 的意义在哪里？
}

/*获取当前中断状态*/
enum intr_status intr_get_status() {
  uint32_t eflags=0;
  GET_EFLAGS(eflags);
  return (EFLAGS_IF & eflags) ? INTR_ON : INTR_OFF;
}

/*完成有关中断的所有初始化工作*/
void idt_init(void) {
  put_str("idt_init start\n");
  idt_desc_init();  // 初始化中断描述符表
  exception_init(); // 异常名初始化并注册通常的中断处理函数
  pic_init();       // 初始化8259A中断控制器

  /*加载idt 75 */
  uint64_t idt_operand = ((sizeof(idt) - 1) | ((uint64_t)(uint32_t)idt << 16));
  asm volatile("lidt %0" : : "m"(idt_operand));
  put_str("idt_init done\n");

}
