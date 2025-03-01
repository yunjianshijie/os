#ifndef __LIB_USER_SYSCALL_H
#define __LIB_USER_SYSCALL_H
#include "stdint.h"
enum SYSCALL_NR
{
    SYS_GETPID,
    SYS_WRITE,
    SYS_MALLOC,
    SYS_FREE
};

uint32_t getpid(void);
/* 把 buf 中 count 个字符写入文件描述符 fd */
uint32_t write(int32_t fd, const void *buf, uint32_t count);
void *malloc(uint32_t size);
void free(void *ptr);
#endif
