#ifndef __LIB_USER_SYSCALL_H__
#define __LIB_USER_SYSCALL_H__
#include "stdint.h"
#include "global.h"
enum SYSCALL_NR
{
    SYS_GETPID
};

uint32_t getpid(void);
#endif