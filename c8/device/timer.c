#include "timer.h"
#include "io.h"
#include "print.h"

#define IRQ0_FREQUENCY 100
#define INPUT_FREQUENCY 1193180
#define COUNTER0_VALUE (INPUT_FREQUENCY / IRQ0_FREQUENCY)
#define CONTER0_PORT 0x40
#define COUNTER0_NO   0
#define COUNTER_MODE 2
#define READ_WRITE_LATCH 3
#define PIT_CONTROL_PORT 0x43

/*把操作的计数器counter_no ,读写锁属性rwl，计数器模式
    * counter_mode 写入模式控制寄存器并赋值 counter_value 
    */
   static void frequency_set(uint8_t counter_port,uint8_t counter_no,uint8_t rwl,uint8_t counter_mode,uint16_t counter_value){
    //往控制字寄存器端口0x43中写入控制字
    outb(PIT_CONTROL_PORT,(uint8_t)((counter_no << 6) | (rwl << 4) | counter_mode << 1));
    //先写counter_value的低8位
    outb(counter_port,(uint8_t)counter_value);
    //再写counter_value的高8位
    outb(counter_port,(uint8_t)(counter_value >> 8));
   }
   
   void timer_init(){
       put_str("timer_init start\n");
       frequency_set(CONTER0_PORT,COUNTER0_NO,READ_WRITE_LATCH,COUNTER_MODE,COUNTER0_VALUE);
       put_str("timer_init done\n");
   }


