#ifndef USERPROG_SYSCALL_INIT_H
#define USERPROG_SYSCALL_INIT_H
#include "stdint.h"
#include "global.h"
// enum SYSCALL_NR { SYS_GETPID };

/* 返回当前任务的pid*/
uint32_t sys_getpid(void);
/* 初始化系统调用*/
void syscall_init(void);


#endif