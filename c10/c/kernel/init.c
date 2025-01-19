#include "init.h"
#include "print.h"
#include "interrupt.h"
#include "timer.h"
#include "memory.h"
#include "thread.h"
#include "console.h"
void init_all(void){
    put_str("init_all\n");
    idt_init(); // 初始化中断
    mem_init(); // 初始化内存管理
    thread_init(); // 初始化线程管理
    timer_init(); // 初始化时钟PIT
    //console_init(); // 初始化控制台，最好放在开中断之前
}