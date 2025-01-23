#ifndef _THREAD_THREAD_H
#define _THREAD_THREAD_H
#include "debug.h"
#include "interrupt.h"
#include "list.h"
#include "print.h"
#include "stdint.h"
/* 自定义通用函数了类型，它将在很多线程函数中作为形参类型*/
typedef void thread_func(void *);
// 作为函数指针？

/* 进程或线程状态*/
enum task_status {
  TASK_RUNNING,
  TASK_READY,
  TASK_BLOCKED,
  TASK_WAITING,
  TASK_HANGING,
  TASK_DEAD
};

/*********** 中断栈 intr_stack ***********
 * 此结构用于中断发生时保护程序（线程或进程）的上下文环境:
 * 进程或线程被外部中断或软中断打断时，会按照此结构压入上下文
 * 寄存器，intr_exit 中的出栈操作是此结构的逆操作
 * 此栈在线程自己的内核栈中位置固定，所在页的最顶端
 ********************************************/
struct intr_stack {
  uint32_t vec_no; // 中断号
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t esp_dummy; // clobbered esp

  // 虽然 pushad 把 esp 也压入，但 esp 是不断变化的，所以会被 popad 忽
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;
  uint32_t gs;
  uint32_t fs;
  uint32_t es;
  uint32_t ds;

  /* 以下由 cpu 从低特权级进入高特权级时压入 */
  uint32_t err_code; // err_code 会被压入在 eip 之后
  void (*eip)(void);
  uint32_t cs;
  uint32_t eflags;
  void *esp;
  uint32_t ss;
};

/* 线程栈thread_stack*
 * 线程自己的栈，用来储存线程中执行的函数
 * 此结构在线程自己的内核栈中位置不固定，
 * 仅用在switch_to时保存线程环境
 * 实际位置取决于实际运行情况。
 ******************************** */
struct thread_stack {
  uint32_t ebp;
  uint32_t ebx;
  uint32_t edi;
  uint32_t esi;
  // 线程第一次执行时，eip指向此函数kernel_thread
  // 其他时候 ，eip 是指向switch_to的返回地址
  void (*eip)(thread_func *func, void *func_arg);
  /***** 以下仅供第一次被调度上 cpu 时使用 ****/
  /* 参数unused_ret 只为占位置充数为返回地址*/
  void(*unused_ret);
  thread_func *function; // 由 kernel_thread 传入
  void *func_arg;        // 由 kernel_thread 传入
};

/* 进程或线程的pcb，程序控制块*/
struct task_struct {
  uint32_t *self_kstack;   // 内核栈地址，栈是向下增长的
                           // 各内核线程都用自己的内核栈
                           //   pid_t pid;              // 进程号
  enum task_status status; // 线程状态
  char name[16];           // 线程名
  uint8_t priority;        // 线程优先级
  uint8_t ticks;           // 每次在处理器上执行的时间嘀嗒数
  /* 此任务自上 cpu 运行后至今占用了多少 cpu 嘀嗒数，
   也就是此任务执行了多久*/
  uint32_t elapsed_ticks;

  /* general_tag 的作用是用于线程在一般的列队中的结点 */
  struct list_elem general_tag;
  /* all_list_tag 的作用是用于线程队列 thread_all_list 中的结点 */
  struct list_elem all_list_tag; // 用于线程队列的结点
  /* */
  uint32_t *pgdir;      // 进程自己页表的虚拟地址
  uint32_t stack_magic; // 栈的边界标记，用于检测栈是否溢出
};

// 在task_struct中添加一个成员，指向下一个task_struct
// 之后是函数指针，用于切换线程
//

// kernel_thread( thread_func *func, void *func_arg){
//     func(func_arg);
//     return 0;
// } // 创建线程
// void init_thread(struct task_struct *pthread, char *name, int prio);

/* 创建一优先级为prio的线程,线程名为name,线程所执行的函数是function(func_arg) */
struct task_struct *thread_start(char *name, int prio, thread_func function,
                                 void *func_arg);
struct task_struct *running_thread();
void schedule(void);
void thread_init(void);
void thread_block(enum task_status stat);
void thread_unblock(struct task_struct *pthread);
#endif