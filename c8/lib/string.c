#include "string.h"
#include "debug.h"
#include "global.h"

#define NULL ((void *)0)

/* 将 dst_起始的 size 个字节置为 value */
void memset(void *dst_, uint8_t value, uint32_t size) {
  ASSERT(dst_ != NULL);
  uint8_t *dst = (uint8_t *)dst_;
  while (size-- > 0) {
    *dst++ = value;
  }
}

/* 将src_起始的size个字节复制到dst_*/
void memcpy(void *dst_, const void *src_, uint32_t size) {
  ASSERT(dst_ != NULL && src_ != NULL);
  uint8_t *dst = (uint8_t *)dst_;
  const uint8_t *src = (const uint8_t *)src_;
  while (size-- > 0) {
    *dst++ = *src++;
  }
}

/*连续比较以地址a_和地址b_开头的size字节，若相等则返回0，若a_大于b_，返回+1,否则返回-1*/
int memcmp(const void *a_, const void *b_, uint32_t size) {
  const uint8_t *a = (const uint8_t *)a_;
  const uint8_t *b = (const uint8_t *)b_;
  while (size-- > 0) {
    if (*a != *b) {
      return *a > *b ? 1 : -1;
    }
    a++;
    b++;
  }
  return 0;
}

/*将字符串从src_复制到dst_*/
char *strcpy(char *dst_, const char *src_) {
  ASSERT(dst_ != NULL && src_ != NULL);
  char *dst = dst_; // 用来返回目的字符串起始地址
  const char *src = src_;
  // while ((*dst++ = *src++) != '\0');
  while ((*dst++ = *src++))
    ;
  return dst_;
}

/*返回字符串长度*/
uint32_t strlen(const char *str) {
  ASSERT(str != NULL);
  const char *p = str;
  while (*p++)
    ;
  return p - str - 1; // 减去1是因为p指向的是'\0'
}

/* 比较两个字符串，若 a_中的字符大于b_中的字符返回1
相等时返回0，否则返回-1 */
int8_t strcmp(const char *a_, const char *b_) {
  ASSERT(a_ != NULL && b_ != NULL);
  char *a = (char *)a_;
  char *b = (char *)b_;
  while (a && b && *a == *b) {
    a++;
    b++;
  }
  return *a < *b ? -1 : (*a > *b);
}

/*从左到右查找字符串str中首次出现字符ch的地址*/
char *strchr(const char *str, const uint8_t ch) {
  ASSERT(str != NULL);
  while (*str != '\0') {
    if (*str == ch) {
      return (char *)str;
    }
    str++;
  }
  return NULL;
}

/*从后往前（右到左）查找字符串str 中首次出现字符ch的地址*/
char *strrchr(const char *str, const uint8_t ch) { 
    ASSERT(str != NULL);
    const char *last_char = NULL;
    /*从头到尾遍历一次，若存在ch字符,last_char总是该字符最后一次
    出现在串中的地址（不是下标，是地址）*/
    while(*str!=0){
        if(*str==ch){
            last_char=str;
        }
    }
    return (char *)last_char;
}

/*将字符串src_拼接到det_后，返回拼接的串地址*/
char *strcat(char *dst_, const char *src_) {
    ASSERT(dst_ != NULL && src_ != NULL);
    char *str = dst_;
    while(*str++);
    str--; // 别看错了，--str 是独立的一句，并不是 while 的循环
    while((*str++ = *src_++))
        ;//当*str被赋值为0时，也就是表达式不成立，正好添加了字符串结尾的0

    return dst_;
}

/* 字符串中str中查找字符ch出现的次数*/
uint32_t strchrs(const char *str ,uint8_t ch){
    ASSERT(str != NULL);
    uint32_t ch_cnt = 0;
    const char *p = str;
    while(*p != '\0'){
        if(*p == ch){
            ch_cnt++;
        }
        p++;
    }
    return ch_cnt;
}

