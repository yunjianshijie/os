#include "ioqueue.h"
#include "debug.h"
#include "interrupt.h"

/* 初始化io队列ioq */
void ioqueue_init(struct ioqueue *ioq) {
  lock_init(&ioq->lock);                // 初始化io队列的锁
  ioq->producer = ioq->consumer = NULL; //  生产者和消费者都为空
  ioq->head = ioq->tail = 0;            // 队列头和队列尾都为0
}

/* 返回pos在缓冲区中在下一个位置值 */
static int32_t next_pos(int pos) { return (pos + 1) % bufsize; }
/* 判断队列是否满*/

// 判断io队列是否已满
bool ioq_full(struct ioqueue *ioq) {
  // 断言中断状态为关闭
  ASSERT(intr_get_status() == INTR_OFF);
  // 判断下一个位置是否等于队尾
  return next_pos(ioq->head) == ioq->tail;
}

/* 判断队列是否已空 */
bool ioq_empty(struct ioqueue *ioq) {
  ASSERT(intr_get_status() == INTR_OFF);
  return ioq->head == ioq->tail;
}

/* 使当前生产者或者消费者在此缓冲区上等待*/
static void ioq_wait(struct task_struct **waiter) {
  ASSERT(*waiter == NULL && waiter != NULL);
  *waiter = running_thread();
  thread_block(TASK_BLOCKED); // 阻塞当前线程
}

/* 唤醒waiter */
static void wakeup(struct task_struct **waiter) {
  ASSERT(*waiter != NULL);
  thread_unblock(*waiter);
  *waiter = NULL;
}

/* 消费者从 ioq队列中获取一个字符*/
char ioq_getchar(struct ioqueue *ioq) {
  ASSERT(intr_get_status() == INTR_OFF);
  /* 如果缓冲区（队列）为空，把消费者ioq->consumer记为当前线程自己
   * 目的就是将来生产者往缓冲区里装商品后，生产者知道唤醒哪个消费者，
   * 也就知道唤醒当前线程自己*/
  while (ioq_empty(ioq)) {
    lock_acquire(&ioq->lock);
    ioq_wait(&ioq->consumer); // 等待队列有内容
    lock_release(&ioq->lock);
  }
  char byte = ioq->buf[ioq->tail]; // 取出队列中的内容
  ioq->tail = next_pos(ioq->tail); // 队列尾指针后移

  if(ioq->producer != NULL) {
    wakeup(&ioq->producer); // 唤醒生产者
  }
  return byte;
}


/* 生产者往ioq队列中写入一个字符byte*/
void ioq_putchar(struct ioqueue *ioq ,char byte) {
    ASSERT(intr_get_status() == INTR_OFF);

    /* 若缓冲区（队列）已经满了，把生产者ioq->producer 记为自己
    * 为的是当缓冲区里的东西被消费者取完后让消费者知道唤醒哪个生产者，
    * 也就是唤醒当前自己*/

    while(ioq_full(ioq)) {
        lock_acquire(&ioq->lock);
        ioq_wait(&ioq->producer); // 等待队列有空间
        lock_release(&ioq->lock);
    }
    ioq->buf[ioq->head] = byte; // 把byte放入队列中
    ioq->head = next_pos(ioq->head); // 队列头指针后移

    if(ioq->consumer != NULL) {
        wakeup(&ioq->consumer); // 唤醒消费者
    }
}