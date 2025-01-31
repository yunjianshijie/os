#ifndef _USERPROG_PROCESS_H
#define _USERPROG_PROCESS_H
#include "console.h"
#include "debug.h"
#include "global.h"
#include "interrupt.h"
#include "io.h"
#include "list.h"
#include "memory.h"
#include "print.h"
#include "string.h"
#include "thread.h"
#define USER_VADDR_START 0x8048000 // 用户进程定的起始地址
#define default_prio 31 // 定义默认的优先级

void process_execute(void *filename, char *name);
void process_activate(struct task_struct *p_thread);
#endif