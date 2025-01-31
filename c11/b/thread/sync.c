// #include "thread.h"
#include "sync.h"
#include "list.h"
#include "interrupt.h"
#include "debug.h"
/* 初始化信号量*/
void sema_init(struct semaphore *psema, uint8_t value) {
  psema->value = value;       // 为信号量赋初值
  list_init(&psema->waiters); // 初始化信号量的等待队列
}

/* 初始化锁 plock*/
void lock_init(struct lock *plock) {
  plock->holder = NULL;            // 持有锁的线程为空
  sema_init(&plock->semaphore, 1); // 初始化信号量，初值为1
  plock->holder_repeat_nr = 0;     // 持有锁的线程重复获得锁的次数
}

/* 信号量down操作*/ // 用于请求对共享资源的访问
void sema_down(struct semaphore *psema) {
  /* 关中断来实现原子操作 */
  enum intr_status old_status = intr_disable();
  while (psema->value == 0) { // 若value为0，表示已被别的thread持有
    // 如果信号量的值为0，则进入等待队列
    ASSERT(!elem_find(&psema->waiters, &running_thread()->general_tag));
    /* 当时线程不应该已在信号量的 waiters队列中*/
    if (elem_find(&psema->waiters, &running_thread()->general_tag)) {
      PANIC("sema_down: thread blocked has been in waiters_list\n");
    }
    /* 如果信号量的值等于0，则当前线程把自己加入该锁的等待队列，如何堵塞自己*/
    list_append(&psema->waiters, &running_thread()->general_tag);
    thread_block(TASK_BLOCKED); // 堵塞自己
  }
  /* 如果value为1 被唤醒后，会执行下面的代码，也就是获得了锁*/
  psema->value--; // 信号量的值减1
  ASSERT(psema->value == 0);
  /* 恢复之前的中断状态*/
  intr_set_status(old_status);
}

/* 信号量的up操作*/ // 用于释放对共享资源的访问
void sema_up(struct semaphore *psema) {
  /* 关中断*/
  enum intr_status old_status = intr_disable();
  ASSERT(psema->value == 0);
  if (!list_empty(&psema->waiters)) {
    // 如果信号量等待队列不为空，则唤醒等待队列中的第一个线程
    struct task_struct *thread_blocked =
        elem2entry(struct task_struct, general_tag, list_pop(&psema->waiters));
    thread_unblock(thread_blocked);
  }
  ++psema->value;
  /* 恢复之前的中断状态*/
  intr_set_status(old_status);
}

// /* 获取锁plock */
// void lock_acquire(struct lock *plock) {
//     /* 排除曾经自己已经持有锁但还没有将其释放的情况*/
//     if(plock->holder != running_thread()) {
//         sema_down(&plock->semaphore); //对信号量P操作，原子操作
//         plock->holder = running_thread(); // 设置持有锁的线程为当前线程
//         ASSERT(plock->holder_repeat_nr == 0);
//         plock->holder_repeat_nr = 1; // 设置持有锁的线程重复获得锁的次数为1
//     }else{
//         plock->holder_repeat_nr++; //
//         如果持有锁的线程是当前线程，则重复获得锁的次数加1
//     }
// }
// /* 释放锁plock */
// void lock_release(struct lock *plock) {
//     ASSERT(plock->holder == running_thread());
//     if(plock->holder_repeat_nr > 1){
//         plock->holder_repeat_nr--; //
//         如果持有锁的线程重复获得锁的次数大于1，则重复获得锁的次数减1 return;
//     }
//     ASSERT(plock->holder_repeat_nr == 1);
//     plock->holder = NULL; // 将持有锁的线程设置为空
//     plock->holder_repeat_nr = 0; // 将持有锁的线程重复获得锁的次数设置为0
//     sema_up(&plock->semaphore); // 对信号量V操作，原子操作
// }

/* 获取锁 */
void lock_acquire(struct lock *plock) {
  if (plock->holder != running_thread()) {
    sema_down(&plock->semaphore); // 对信号P操作，原子操作
    plock->holder = running_thread();
    ASSERT(plock->holder_repeat_nr == 0);
    plock->holder_repeat_nr = 1;
  } else {
    ++plock->holder_repeat_nr;
  }
}

/* 释放锁 */
void lock_release(struct lock *plock) {
  ASSERT(plock->holder == running_thread());
  if (plock->holder_repeat_nr > 1) {
    --plock->holder_repeat_nr;
    return;
  }
  ASSERT(plock->holder_repeat_nr == 1);
  plock->holder = NULL; // 把锁的持有者置空放在V操作之前
  plock->holder_repeat_nr = 0;
  sema_up(&plock->semaphore); // 信号量的V操作，也是原子操作
}




