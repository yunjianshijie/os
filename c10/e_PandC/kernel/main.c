// #include "ioqueue.h"
// #include "../device/keyboard.h"
// #include "console.h"
// #include "debug.h"
// #include "init.h"
// #include "memory.h"
// #include "print.h"
// #include "thread.h"
// #include "timer.h"
// #include "interrupt.c"

#include "console.h"
#include "init.h"
#include "interrupt.h"
#include "print.h"
#include "thread.h"

/* 临时为测试添加 */
#include "ioqueue.h"
#include "keyboard.h"
void k_thread_a(void *);
void k_thread_b(void *);
int main(void) {
  put_str("I am kernel\n");
  init_all();

  thread_start("k_thread_a", 31, k_thread_a, "A_");
  thread_start("k_thread_b", 8, k_thread_b, "B_");
  intr_enable(); // 打开中断,使时钟中断起作用
  while (1) {
    // console_put_str("Main ");
  }
  //   ;
  // asm volatile("sti"); // 为演示中断处理，在此临时开中断
  //  while(1)
  // ;
  return 0;
}

void k_thread_a(void *arg) {
  char *para = arg;
  while (1) {
    enum intr_status old_status = intr_disable();
    ioq_empty(&kbd_buf);
    // if (!ioq_empty(&kbd_buf)) {
    //   console_put_str(para);
    //   char byte = ioq_getchar(&kbd_buf);
    //   console_put_char(byte);
    // }
    intr_set_status(old_status);
    // console_put_str(arg);
  }
  return;
}
void k_thread_b(void *arg) {
  // char *para = arg;
  // while (1) {
  //   enum intr_status old_status = intr_disable();
  //   if (!ioq_empty(&kbd_buf)) {
  //     console_put_str(para);
  //     char byte = ioq_getchar(&kbd_buf);
  //     console_put_char(byte);
  //   }
  //   intr_set_status(old_status);
  //   // console_put_str(arg);
  // }
  return;
}