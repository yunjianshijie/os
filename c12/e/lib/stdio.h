#ifndef __LIB_STDIO_H
#define __LIB_STDIO_H
#include "global.h"
#include "stdint.h"
/* 将整型转换成字符（integer to ascii）*/
static void itoa(uint32_t value, char **buf_ptr_addr, uint8_t base);
/* 将参数ap按照格式format输出到字符str，并返回替换str长度 */
uint32_t vsprintf(char *str, const char *format, va_list ap);
/* 格式化输出字符串format*/
uint32_t printf(const char *format, ...);
#endif