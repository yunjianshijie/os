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
// idx-->index
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

// #include "bitmap.h" //不仅是为了通过一致性检查，位图的数据结构struct bitmap也在这里面
// #include "debug.h"  //ASSERT
// #include "interrupt.h"
// #include "print.h"
// #include "stdint.h"
// #include "string.h" //里面包含了内存初始化函数，memset

// // 将位图btmp初始化
// void bitmap_init(struct bitmap *btmp) {
//   memset(btmp->bits, 0, btmp->btmp_bytes_len);
// }
// // 用来确定位图的某一位是1，还是0。若是1，返回真（返回的值不一定是1）。否则，返回0。传入两个参数，指向位图的指针，与要判断的位的偏移
// bool bitmap_scan_test(struct bitmap *btmp, uint32_t bit_idx) {
//   uint32_t byte_idx = bit_idx / 8; // 确定要判断的位所在字节的偏移
//   uint32_t bit_odd = bit_idx % 8; // 确定要判断的位在某个字节中的偏移
//   return (btmp->bits[byte_idx] & (BITMAP_MASK << bit_odd));
// }
// // 用来在位图中找到cnt个连续的0，以此来分配一块连续未被占用的内存，参数有指向位图的指针与要分配的内存块的个数cnt
// // 成功就返回起始位的偏移（如果把位图看做一个数组，那么也可以叫做下标），不成功就返回-1
// int bitmap_scan(struct bitmap *bitmap, uint32_t cnt) {
//   uint32_t area_start = 0, area_size = 0; // 用来存储一个连续为0区域的起始位置,
//                                           // 存储一个连续为0的区域大小
//   while (1) {
//     while (bitmap_scan_test(bitmap, area_start) &&
//            area_start / 8 < bitmap->btmp_bytes_len)
//       // 当这个while顺利结束       1、area_start就是第一个0的位置；
//       // 2、area_start已经越过位图边界
//       area_start++;
//     if (area_start / 8 >= bitmap->btmp_bytes_len)
//       // 上面那个循环跑完可能是area_start已经越过边界，说明此时位图中是全1，那么就没有可分配内存
//       return -1;
//     area_size =
//         1; // 来到了这一句说明找到了位图中第一个0，那么此时area_size自然就是1
//     while (area_size < cnt) {
//       if ((area_start + area_size) / 8 <
//           bitmap->btmp_bytes_len) { // 确保下一个要判断的位不超过边界
//         if (bitmap_scan_test(bitmap, area_start + area_size) ==
//             0) // 判断区域起始0的下一位是否是0
//           area_size++;
//         else
//           break; // 进入else，说明下一位是1，此时area_size还没有到达cnt的要求，且一片连续为0的区域截止
//       } else
//         return -1; // 来到这里面，说面下一个要判断的位超过边界，且area_size<cnt，返回-1
//     }
//     if (area_size == cnt) // 有两种情况另上面的while结束，1、area_size ==
//                           // cnt；2、break；所以要判断
//       return area_start;
//     area_start +=
//         (area_size + 1); // 更新area_start，判断后面是否有满足条件的连续0区域
//   }
// }
// // 将位图某一位设定为1或0，传入参数是指向位图的指针与这一位的偏移，与想要的值
// void bitmap_set(struct bitmap *btmp, uint32_t bit_idx, int8_t value) {
//   ASSERT((value == 0) || (value == 1));
//   uint32_t byte_idx = bit_idx / 8; // 确定要设置的位所在字节的偏移
//   uint32_t bit_odd = bit_idx % 8; // 确定要设置的位在某个字节中的偏移

//   // 一般都会用个0x1这样的数对字节中的位操作,将1任意移动后再取反,或者先取反再移位,可用来对位置0操作
//   if (value)
//     btmp->bits[byte_idx] |= (BITMAP_MASK << bit_odd);
//   else
//     btmp->bits[byte_idx] &= ~(BITMAP_MASK << bit_odd);
// }