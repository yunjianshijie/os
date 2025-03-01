#include "init.h"
#include "console.h"
#include "ide.h"
#include "interrupt.h"
#include "keyboard.h"
#include "memory.h"
#include "print.h"
#include "syscall-init.h"
#include "thread.h"
#include "timer.h"
#include "tss.h"
#include "fs.h"
/*负责初始化所有模块 */
void init_all() {
  put_str("init_all\n");
  idt_init();    // 初始化中断
  mem_init();    // 初始化内存管理系统
  thread_init(); // 初始化线程相关结构
  timer_init();
  console_init();
  keyboard_init();
  tss_init();
  syscall_init();
  ide_init();
  filesys_init();
}
