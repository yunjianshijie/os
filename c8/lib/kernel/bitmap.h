#ifndef _LIB_KERNEL_BITMAP_H
#define _LIB_KERNEL_BITMAP_H
#include "global.h"

// 用来在位图中逐位判断,主要就是通过按位与‘&’来判断相应位是否为 1。
#define BITMAP_MASK 1

//位图结构体定义
struct bitmap
{
    // 位图的字节长度
    uint32_t btmp_bytes_len;
    /* 在遍历位图时，整体上以字节为单位，细节上是以位单位，
    所以此处位图的指针必须是单字节*/
    // 位图的指针
    uint8_t *bits;
    // bits 的类型是 uint8_t*，此类型强调的是字节型指针，最好不要用多字节类型，否则在处理时会复杂。
};
// 初始化位图
void bitmap_init(struct bitmap *btmp);
// 判断bit_idx位是否为1，若为1,则返回true,否则返回false
bool bitmap_scan_test(struct bitmap *btmp, uint32_t bit_idx);
// 在位图中申请连续 cnt 个位，成功，则返回其起始位下标，失败，返回−1
int bitmap_scan(struct bitmap *btmp, uint32_t cnt);
//
void bitmap_set(struct bitmap *btmp, uint32_t bit_idx, int8_t value);

#endif