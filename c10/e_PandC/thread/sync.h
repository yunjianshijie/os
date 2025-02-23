#ifndef _THREAD_SYNC_H
#define _THREAD_SYNC_H
#include "list.h"
#include "stdint.h"
#include "thread.h"
/* 信号量结构*/
struct semaphore {
  uint8_t
      value; // 信号量的值（对信号量进行down 操作时，如果信号量为0就会阻塞线程）
  struct list waiters; /// 记录再此信号量上等待（阻塞）的所有线程
};

/* 锁结构*/
struct lock {
  struct task_struct *holder; // 锁的持有者
  struct semaphore semaphore; // 用二元信号量实现锁
  uint32_t holder_repeat_nr;  // 持有者重复申请锁的次数
};
// 谁成功申请了锁，谁就是锁的持有者

// void lock_init(struct lock *plock);
// /* 获取锁plock */
// void lock_acquire(struct lock *plock);
// /* 释放锁plock */
// void lock_release(struct lock *plock);

// void sema_init(struct semaphore *psema, uint8_t value);
void sema_init(struct semaphore *psema, uint8_t value);
void lock_init(struct lock *lock);
void sema_down(struct semaphore *psema);
void sema_up(struct semaphore *psema);
void lock_acquire(struct lock *plock);
void lock_release(struct lock *plock);
#endif