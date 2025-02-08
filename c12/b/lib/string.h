#ifndef __LIB_STRING_H
#define __LIB_STRING_H
#include "stdint.h"

// 将 `dst_` 指向的内存区域的前 `size` 个字节设置为常数值 `value`。
void memset(void *dst_, uint8_t value, uint32_t size);

// 从 `src_` 复制 `size` 个字节到 `dst_`。源和目标区域不应重叠。
void memcpy(void *dst_, const void *src_, uint32_t size);

// 比较 `a_` 和 `b_` 指向的前 `size` 个字节。
int memcmp(const void *a_, const void *b_, uint32_t size);

// 将 `src_` 指向的字符串复制到 `dst_` 指向的缓冲区，并返回指向 `dst_` 的指针。
char *strcpy(char *dst_, const char *src_);

// 返回 `str` 指向的字符串的长度（不包括终止符 '\0'）。
uint32_t strlen(const char *str);

// 比较两个字符串 `a` 和 `b`，如果相等返回 0，否则返回非零值指示顺序。
int8_t strcmp(const char *a, const char *b);

// 在 `string` 中查找第一个 `ch` 字符的位置，并返回指向该字符的指针，如果没有找到则返回 NULL。
char *strchr(const char *string, const uint8_t ch);

// 在 `string` 中查找最后一个 `ch` 字符的位置，并返回指向该字符的指针，如果没有找到则返回 NULL。
char *strrchr(const char *string, const uint8_t ch);

// 将 `src_` 指向的字符串连接到 `dst_` 指向的字符串的末尾，并返回指向 `dst_` 的指针。
char *strcat(char *dst_, const char *src_);

// 统计 `filename` 中字符 `ch` 出现的次数。
uint32_t strchrs(const char *filename, uint8_t ch);

#endif // __LIB_STRING_H