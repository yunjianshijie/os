#ifndef __KERNEL_KEYBOARD_H
#define __KERNEL_KEYBOARD_H

#include "global.h"
#include "interrupt.h"
#include "io.h"
#include "print.h"
#include "stdint.h"
#define KBD_BUF_PORT 0x60 // 键盘 buffer 寄存器端口号为 0x60

/* 键盘中断处理程序 */
static void intr_keyboard_handler(void);
/* 键盘初始化 */
void keyboard_init(void);
extern struct ioqueue kbd_buf;
#endif