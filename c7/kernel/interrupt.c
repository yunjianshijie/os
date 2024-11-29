#include "interrupt.h"
#include "stdint.h"
#include "global.h"


# define IDT_DESCR_CNT 0x21

/* 中断门描述符结构体*/
struct gate_desc // 结构体中位置越偏下的成员，其地址越高。
{
  uint16_t func_offset_low_word; //中断处理程序在目标代码段内偏移量
  uint16_t selector;  //中断处理程序目标代码段描述符选择子
  //低32位
  uint8_t dcount; // 此项为双字计数字段，是门描述符中的第 4 字节
                  // 此项固定值，不用考虑
  uint8_t attribute; //属性
  uint16_t func_offset_high_word; //中断处理程序在目标段内的偏移量高16位
};

//静态函数声明，非必须
static void make_idt_desc(struct gate_desc *p_gate, uint8_t attr,intr_handler function);


static struct gate_desc idt[IDT_DESCR_CNT];     // idt是中断描述符表
                                                // 本质上就是个中断门描述符

extern intr_handler intr_entry_table[IDT_DESCR_CNT]; // 声明中断处理程序数组 定义在kernel.S中的中断函数入口数组



////////////////////

/* 创建中断门描述符 */
static void make_idt_desc(struct gate_desc *p_gdesc, uint8_t attr , intr_handller function){
p_gdesc->func_offset_low_word = (uint32_t) function & 0x0000FFFF;
p_gdesc->selector = SELECTOR_K_CODE; //内核代码段选择子 。selector是段选择子
p_gdesc->dcount = 0;
p_gdesc->attribute = attr;
p_gdesc->func_offset_high_word = ((uint32_t) function & 0xFFFF0000) >> 16;
}
/*初始化中断描述符表*/

static void idt_desc_init(void){
    int i;
    for(i = 0; i < IDT_DESCR_CNT; i++){ //0x21个中断（32）
        make_idt_desc(&idt[i], IDT_DESCR_ATTR_32_INT, intr_entry_table[i]); //初始化idt每一个中断门描述符
    }
    put_str("  idt_desc_init done\n");
}

/*完成有关中断的所有初始化工作*/
void idt_init(void){
    put_str("idt_init start\n");
    idt_desc_init();        // 初始化中断描述符表
    pic_init();             //初始化8259A中断控制器

    /*加载idt*/
    uint64_t idt_operamd = ((sizeof(idt) - 1) | ((uint64_t)(uint32_t)idt << 16));
    asm volatile("lidt %0" : : "m"(idt_operand));
    put_str("idt_init done\n");
}
