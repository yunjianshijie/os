#include "thread.h"
#include "../kernel/memory.h"
#include "global.h"
#include "stdint.h"
#include "string.h"

#define PG_SIZE 4096

/* 由kernel_thread去执行 funvtion(func_arg)*/

static void kernel_thread(thread_func *function, void *func_arg) {
  function(func_arg);
}

/* 初始化线程栈thread_stack,
 * 将带执行的函数和参数放到thread_stack中相应的位置*/
void thread_create(struct task_struct *pthread, thread_func function,
                   void *func_arg) {
  /*  先预留中断使用栈的空间，可见 thread.h 中定义的结构*/
  pthread->self_kstack -= sizeof(struct intr_stack); // 添加中断栈
  // 内核栈地址，栈是向下增长的，所以要减去
  /* 再预留出线程栈空间 */
  pthread->self_kstack -= sizeof(struct thread_stack);
  struct thread_stack *kthread_stack =
      (struct thread_stack *)pthread->self_kstack;
  kthread_stack->eip = kernel_thread; // 函数指针

  kthread_stack->function = function; // 函数指针
  kthread_stack->func_arg = func_arg; // 函数参数
  kthread_stack->ebp = kthread_stack->ebx = kthread_stack->esi =
      kthread_stack->edi = 0;
  // 初始化寄存器
}

void init_thread(struct task_struct *pthread, char *name, int prio) {
    // 线程pcb ,名字，优先级
  memset(pthread, 0, sizeof(*pthread));
  //先把线程pcb清零
  strcpy(pthread->name, name); //把名字复制到线程pcb中
  pthread->status = TASK_RUNNING; // 0 task运行，表示线程正在运行或准备运行
  pthread->priority = prio;       // 优先级
  /* self_kstack 是线程自己在内核态下使用的栈顶地址 */
  pthread->self_kstack = (uint32_t*)((uint32_t)pthread + PG_SIZE);  // PG_SIZE=4kb，一个页的大小 栈开始的地方就等于线程开始地方+4kb
  pthread->stack_magic = 0x19870916; // 魔术数，用于检测栈是否溢出
}
/* 创建一优先级为 prio 的线程，线程名为 name，
 线程所执行的函数是 function(func_arg) */

struct task_struct *thread_start(char *name, int prio, thread_func function,
                                 void *func_arg) {
  /* pcb 都位于内核空间，包括用户进程的pcb也是在内核空间*/
  struct task_struct *thread = get_kernel_pages(1);// 所以申请一个内核空间
  init_thread(thread, name, prio);// 初始化线程
  thread_create(thread, function, func_arg);
  asm volatile(
      "movl %0, %%esp\n  pop %%ebp\n pop %%ebx\n pop %%edi\n pop %%esi\n ret"
      :
      : "g"(thread->self_kstack)
      : "memory");
  return thread;
}