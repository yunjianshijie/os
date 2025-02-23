#ifndef __DEVICE_IOQUEUE_H
#define __DEVICE_IOQUEUE_H
#include "stdint.h"
#include "sync.h"
#include "thread.h"
#include <stdbool.h>
#define bufsize 64
/* 环形队列*/
struct ioqueue {
  // 生产者消费者问题
  struct lock lock;
  /* 生产者，缓冲区不满时就继续往里面放数据，
   * 否则就睡眠，此项记录哪个生产者在此缓冲区上睡眠 */
  struct task_struct *producer;
  /* 消费者，缓冲区不空时就继续从里面拿数据，
   * 否则就睡眠，此项记录哪个消费者在此缓冲区上睡眠 */
  struct task_struct *consumer;
  char buf[bufsize]; // 缓冲区大小
  int32_t head;      // 队首，数据往队首处写入
  int32_t tail;      // 队尾，数据往队尾处读出
};

/* 初始化io队列ioq */
void ioqueue_init(struct ioqueue *ioq);
/* 返回pos在缓冲区中在下一个位置值 */
static int32_t next_pos(int pos);
// 判断io队列是否已满
bool ioq_full(struct ioqueue *ioq);
/* 判断队列是否已空 */
bool ioq_empty(struct ioqueue *ioq);
/* 使当前生产者或者消费者在此缓冲区上等待*/
static void ioq_wait(struct task_struct **waiter);
/* 唤醒waiter */
static void wakeup(struct task_struct **waiter);
/* 消费者从 ioq队列中获取一个字符*/
char ioq_getchar(struct ioqueue *ioq);
/* 生产者往ioq队列中写入一个字符byte*/
void ioq_putchar(struct ioqueue *ioq, char byte);
#endif