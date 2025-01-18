#include "console.h"
#include "debug.h"
#include "init.h"
#include "memory.h"
#include "print.h"
#include "thread.h"
#include "timer.h"
void k_thread_a(void *);
void k_thread_b(void *);
int main(void) {
  put_str("I am kernel\n");
  init_all();
  thread_start("k_thread_a", 31, k_thread_a, "argA1 ");
  thread_start("k_thread_b", 8, k_thread_b, "argB2 ");
  intr_enable(); // 打开中断,使时钟中断起作用
  intr_enable(); // 打开中断，使时钟中断起作用
  while (1) {
   // console_put_str("Main ");
  }
  return 0;
}

void k_thread_a(void *arg) {
  char *para = arg;
  while (1) {
  // console_put_str(para);
  }
  return;
}
void k_thread_b(void *arg) {
  char *para = arg;
  while (1) {
  //  console_put_str(para);
  }
  return;
}