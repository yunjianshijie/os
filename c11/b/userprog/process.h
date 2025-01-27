#ifndef _USERPROG_PROCESS_H
#define _USERPROG_PROCESS_H
#include "global.h"
#include "interrupt.h"
#include "io.h"
#include "list.h"
#include "memory.h"
#include "print.h"
#include "string.h"
#include "thread.h"


void tss_init(void);
#endif