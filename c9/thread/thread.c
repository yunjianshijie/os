#include "thread.h"
#include "../kernel/memory.h"
#include "global.h"
#include "stdint.h"
#include "string.h"
#include "list.h"

#define PG_SIZE 4096

struct task_struct *main_thread;     // 主线程 pcb
struct list thread_ready_list;       // 就绪队列
struct list thread_all_list;         // 所有线程队列
static struct list_elem *thread_tag; //  用于保存队列中的线程结点

extern void switch_to(struct task_struct *cur, struct task_struct *next);

/* 获取当前线程pcb指针*/
struct task_struct *running_thread() {
  uint32_t esp;
  asm volatile("mov %%esp, %0" : "=g"(esp));
  // 取esp 整数部分，即pcb起始地址
  return (struct task_struct *)(esp & 0xfffff000); // 前15位
}

/* 由kernel_thread去执行 funvtion(func_arg)*/
static void kernel_thread(thread_func *function, void *func_arg) {
  /* 执行function 前要开中断，
   * 避免后面的时钟中断被屏蔽，而无法调度其他线程*/
  intr_enable();
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

/* 初始化线程基本信息*/
void init_thread(struct task_struct *pthread, char *name, int prio) { //prio优先级
  // 线程pcb ,名字，优先级
  memset(pthread, 0, sizeof(*pthread));
  // 先把线程pcb清零

  strcpy(pthread->name, name); // 把名字复制到线程pcb中
  if (pthread == main_thread) {
    /* 由于把main函数也封装成一个线程，
    并且它是一直是运行的，故将其直接设为TASK_RUNNING*/
    pthread->status = TASK_RUNNING; // 0 task运行，表示线程正在运行或准备运行
  } else {
    pthread->status = TASK_READY; // 1 task就绪，表示线程已就绪，等待被调度
  }

  /* self_kstack 是线程自己在内核态下使用的栈顶地址 */
  pthread->self_kstack =
      (uint32_t *)((uint32_t)pthread +
                   PG_SIZE); // PG_SIZE=4kb，一个页的大小
                             // 栈开始的地方就等于线程开始地方+4kb
  pthread->priority = prio;
  pthread->ticks = prio;             // 时间片
  pthread->elapsed_ticks = 0;        // 线程执行的时间
  pthread->pgdir = NULL;             // 线程页表
  pthread->stack_magic = 0x19870916; // 魔术数，用于检测栈是否溢出
}
/* 创建一优先级为 prio 的线程，线程名为 name，
 线程所执行的函数是 function(func_arg) */
struct task_struct *thread_start(char *name, int prio, thread_func function,
                                 void *func_arg) {
  /* pcb 都位于内核空间，包括用户进程的pcb也是在内核空间*/
  struct task_struct *thread = get_kernel_pages(1); // 所以申请一个内核空间
  init_thread(thread, name, prio);                  // 初始化线程
  thread_create(thread, function, func_arg);
  /* 确保之前不在队列中*/
  ASSERT(!elem_find(&thread_ready_list, &thread->general_tag));
  /* 加入就绪线程队列中*/
  list_append(&thread_ready_list, &thread->general_tag);
  /* 确保之前不在对垒中*/
  ASSERT(!elem_find(&thread_all_list, &thread->general_tag));
  /* 加入所有线程队列中*/
  list_append(&thread_all_list, &thread->general_tag);
  // asm volatile(
  //     "movl %0, %%esp\n  pop %%ebp\n pop %%ebx\n pop %%edi\n pop %%esi\n ret"
  //     :
  //     : "g"(thread->self_kstack)
  //     : "memory");
  return thread;
}

/* 将 kernel 中的 main 函数完善为主线程 */
static void make_main_thread(void) {
  /* 因为main线程早就运行了
   * 咱在loader.S中进入内核时的mov esp ，0xc009f000,1111
   * 就是为其预留pcb的，因此pcb地址为0xc009e000,    1110
   * 不需要通过get_kernel_page另分配一页*/
  main_thread = running_thread();
  init_thread(main_thread, "main", 31);//31表示最高优先级 0-31
  /* main 函数是当前线程，当前线程不在 thread_ready_list中，
   * 所以只将其加在thread_all_list 中*/
  ASSERT(!elem_find(&thread_all_list, &main_thread->general_tag));
  list_append(&thread_all_list, &main_thread->all_list_tag);
}