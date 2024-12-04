#include "bitmap.h"
#include "debug.h"
#include "interrupt.h"
#include "memory.h"
#include "stdint.h"
#include "string.h"

/* 将位图btmp初始化*/
void bitmap_init(struct bitmap *btmp) {
  // 把bitmap的位全部初始化为0
  memset(btmp->bits, 0, btmp->btmp_bytes_len);
}
// 判断bit_idx位是否为1，若为1,则返回true,否则返回false
bool bitmap_scan_test(struct bitmap *btmp, uint32_t bit_idx) {
  uint32_t byte_idx = bit_idx / 8; // 向下取整用于索引数组下标 (有多少字节)
  uint32_t bit_idx_offset =
      bit_idx % 8; // 用于索引数组内的位 （一个字节的第几位）
  return (btmp->bits[byte_idx] & (BITMAP_MASK << bit_idx_offset)) != 0;
  // btmp->bits[byte_idx] & 00000001运算，如果为1则返回true，否则返回false
  // ,btmp->bits[byte_idx]是这个
}
// 在位图中申请连续 cnt 个位，成功，则返回其起始位下标，失败，返回−1
int bitmap_scan(struct bitmap *btmp, uint32_t cnt) {
  uint32_t idx_byte = 0; // 用于记录空闲位所在的字节 32 4位
  // 逐字节比较，用蛮力
  while ((idx_byte < btmp->btmp_bytes_len) &&
         (0xff == btmp->bits[idx_byte])) // 0xff表示该字节内已无空闲位
  {
    /* 1. 表示该位已分配，如果是0xff，则表示该字节内已无空闲位，向下一字节继续找
     */
    idx_byte++;
  }
  ASSERT(idx_byte < btmp->btmp_bytes_len);
  // 找到了空字节
  if (idx_byte == btmp->btmp_bytes_len) {
    return -1;
  }
  /* 如果位图数组范围内的某个字节内找到了空闲位
   * 在该字节内逐位对比找到空闲索引*/
  int idx_bit = 0;
  /*和btmp->bits[idx_byte]逐位比较，找到空闲位*/
  while ((uint8_t)(BITMAP_MASK << idx_bit) & btmp->bits[idx_byte]) {
    idx_bit++;
  }
  // 找到了空闲位 idx_b
  int bit_idx_start = idx_byte * 8 + idx_bit;
  if (cnt == 1) {
    return bit_idx_start;
  }
  uint32_t bit_left = (btmp->btmp_bytes_len * 8 - bit_idx_start);
  // 记录还有多少位可以判断
  uint32_t next_bit = bit_idx_start + 1;
  uint32_t count = 1; // 记录找到的空闲位的个数

  bit_idx_start = -1; // 如果先将其置为-1，如果找不到就可以直接返回
  while (bit_left-- > 0) {
    if (!(bitmap_scan_test(btmp, next_bit))) {
      count++;
    } else {
      count = 0;
    }
    if (count == cnt) { // 若找到连续的cnt个空闲位
      bit_idx_start = next_bit - cnt + 1;
      break;
    }
    next_bit++;
  }
  return bit_idx_start;
}
//idx-->index
/* 将位图 btmp 的 bit_idx 位设置为 value */
void bitmap_set(struct bitmap *btmp, uint32_t bit_idx, int8_t value) {
  ASSERT((value == 0) || (value == 1));
  uint32_t byte_idx = bit_idx / 8; // 向下取整用于索引数组下标 (有多少字节)
  uint32_t bit_old = bit_idx % 8; // 取余用于索引数组内的位
/* 一般都会用0x01这样数字对字节中的位操作
* 将1任意移动后再取反，或者先取反再移动，可以用于对0操作*/
  if (value) { // 如果 value 为 1
    btmp->bits[byte_idx] |= (BITMAP_MASK << bit_old);
  } else {
    btmp->bits[byte_idx] &= ~(BITMAP_MASK << bit_old);
  }
  /* 一般都会用于0x1这样的数对字节的位操作
   * 将1任意移动后再取反，或者先*/
}
