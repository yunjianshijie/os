#include "print.h"
#include "init.h"
#include "timer.h"
#include "debug.h"
#include "memory.h"
int main(void) {
  put_str("I am kernel\n");
  init_all();
  void * addr = get_kernel_pages(3);
  //void * addr2 = get_kernel_pages(3);
  put_str("\n get_kernel_page start vaddr is ");
  put_int((uint32_t)addr);
  // put_str("\n get_kernel_page end vaddr is ");
  // put_int((uint32_t)addr2);
  put_str("\n");

  //ASSERT(1 == 2); // 测试断言
  while(1)
    ;
  //asm volatile("sti"); // 为演示中断处理，在此临时开中断
  return 0;
}