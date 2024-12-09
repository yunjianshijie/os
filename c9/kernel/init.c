#include "init.h"
#include "print.h"
#include "interrupt.h"
#include "timer.h"
#include "memory.h"
#include "thread.h"
void init_all(void){
    put_str("init_all\n");
    idt_init(); // 初始化中断
    timer_init(); // 初始化时钟PIT
    mem_init(); // 初始化内存管理
    //init_thread(); // 初始化线程管理
}