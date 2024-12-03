#include "print.h"
#include "init.h"
#include "timer.h"
#include "debug.h"
void main(void) {
  put_str("I am kernel\n");
  init_all();
  ASSERT(1 == 2); // 测试断言
  while(1)
    ;
  //asm volatile("sti"); // 为演示中断处理，在此临时开中断
  // while (1)
  //   ;
  }