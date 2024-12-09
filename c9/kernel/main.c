#include "print.h"
#include "init.h"
#include "timer.h"
#include "debug.h"
#include "memory.h"
#include "thread.h"
void k_thread_a(void *);
int main(void) {
  put_str("I am kernel\n");
  init_all();

  // void * addr = get_kernel_pages(3);
  // put_str("\n get_kernel_page start vaddr is ");
  // put_int((uint32_t)addr);

  thread_start("k_thread_a", 31, k_thread_a, "argA1");

  //put_str("\n");
  //ASSERT(1 == 2); // 测试断言
  while(1)
    ;
  //asm volatile("sti"); // 为演示中断处理，在此临时开中断
  return 0;
}

void k_thread_a(void * arg){
  char *para = arg;
  while(1){
    put_str(para);
  }
  return;
}