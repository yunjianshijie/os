#ifndef _USERPROG_TSS_H
#define _USERPROG_TSS_H
#include "global.h"
#include "interrupt.h"
#include "io.h"
#include "list.h"
#include "memory.h"
#include "print.h"
#include "string.h"
#include "thread.h"

#define PG_SIZE 4096
void tss_init(void);
#endif