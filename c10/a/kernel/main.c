#include "print.h"
#include "init.h"
#include "timer.h"
#include "debug.h"
#include "memory.h"
#include "thread.h"
void k_thread_a(void *);
void k_thread_b(void *);
int main(void) {
  put_str("I am kernel\n");
  init_all();
  // void * addr = get_kernel_pages(3);
  // put_str("\n get_kernel_page start vaddr is ");
  // put_int((uint32_t)addr);
  
  thread_start("k_thread_a", 31, k_thread_a, "argA1 ");
  thread_start("k_thread_b", 8, k_thread_b, "argB2 ");
  intr_enable(); // 打开中断,使时钟中断起作用
  //put_str("\n");
  //ASSERT(1 == 2); // 测试断言
  //intr_enable(); // 打开中断，使时钟中断起作用
  while(1){
    intr_disable();
    put_str("main ");
    intr_enable();
  }
  //   ;
  //asm volatile("sti"); // 为演示中断处理，在此临时开中断
//  while(1)
    ;
  return 0;
}

void k_thread_a(void * arg){
  char *para = arg;
  while(1){
    intr_disable();
    put_str(para);
    intr_enable();
  }
  return;
}
void k_thread_b(void *arg) {
  char *para = arg;
  while (1) {
    intr_disable();
    put_str(para);
    intr_enable();
  }
  return;
  
}