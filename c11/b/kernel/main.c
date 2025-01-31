#include "print.h"
#include "init.h"
#include "timer.h"
#include "debug.h"
//#include "memory.h"
#include "thread.h"
#include "console.h"

#include "process.h"
/* 临时为测试添加 */
#include "ioqueue.h"
#include "keyboard.h"

int tast_var_a =0, tast_var_b = 0;

void k_thread_a(void *);
void k_thread_b(void *);
void u_prog_a(void);
void u_prog_b(void);
int main(void) {
  put_str("I am kernel\n");
  init_all();

  thread_start("k_thread_a", 31, k_thread_a, "A_");
  thread_start("k_thread_b", 8, k_thread_b, "B_");
  process_execute(u_prog_a, "user_prog_a");
  process_execute(u_prog_b, "user_prog_b");
  intr_enable(); // 打开中断,使时钟中断起作用

  while(1){
    //console_put_str("Main ");
  }
 
//  while(1) ;
  return 0;
}

void k_thread_a(void * arg){
  char *para = arg;
  while (1) {
    console_put_str("v_a:0x");
    console_put_int(tast_var_a);
  }
  return;
}
void k_thread_b(void *arg) {
  char *para = arg;
  while(1){
    console_put_str("v_b:0x");
    console_put_int(tast_var_b);
  }
  return;
  
}

void u_prog_a(void) {
  while (1)
  {
    tast_var_a += 1;
  }
  
}

void u_prog_b(void) {
  while (1)
  {
    tast_var_b += 1;
  } 
}