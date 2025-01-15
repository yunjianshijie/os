#include "timer.h"

#define IRQ0_FREQUENCY 100
#define INPUT_FREQUENCY 1193180
#define COUNTER0_VALUE (INPUT_FREQUENCY / IRQ0_FREQUENCY)
#define CONTER0_PORT 0x40
#define COUNTER0_NO 0
#define COUNTER_MODE 2
#define READ_WRITE_LATCH 3
#define PIT_CONTROL_PORT 0x43

/*把操作的计数器counter_no ,读写锁属性rwl，计数器模式
 * counter_mode 写入模式控制寄存器并赋值 counter_value
 */
uint32_t ticks; // ticks 是内核自中断开启以来总共的嘀嗒数

static void frequency_set(uint8_t counter_port, uint8_t counter_no, uint8_t rwl,
                          uint8_t counter_mode, uint16_t counter_value) {
  // 往控制字寄存器端口0x43中写入控制字
  outb(PIT_CONTROL_PORT,
       (uint8_t)((counter_no << 6) | (rwl << 4) | counter_mode << 1));
  // 先写counter_value的低8位
  outb(counter_port, (uint8_t)counter_value);
  // 再写counter_value的高8位
  outb(counter_port, (uint8_t)(counter_value >> 8));
}

void timer_init() {
  put_str("timer_init start\n");
  frequency_set(CONTER0_PORT, COUNTER0_NO, READ_WRITE_LATCH, COUNTER_MODE,
                COUNTER0_VALUE);
  register_headler(0x20, intr_timer_handler); //register_headler这个函数在interrupt.c中
  //注册中断处理函数 0x20中断号 intr_timer_handler处理函数

  put_str("timer_init done\n");
}

/* 时钟的中断处理函数 */
static void intr_timer_handler(void) {
    struct task_struct *cur_thread = running_thread(); // cur 当前线程 返回线程pcb
    /* 时钟的中断处理函数*/
    ASSERT(cur_thread->stack_magic == 0x19870916); // 查看是否出栈
    cur_thread->elapsed_ticks++; // 线程运行的时间片+1
    ticks++; // ticks 是内核自中断开启以来总共的嘀嗒数+1
    if(cur_thread->ticks == 0){
        schedule(); // 调度  在thread.c中
    }else{
        cur_thread->ticks--; // 当前线程的ticks-1
    }
}