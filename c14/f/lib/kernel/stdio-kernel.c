#include "stdio-kernel.h"
#include "../stdio.h"
#include "../stdint.h"
#include "console.h"
/* 供内核使用的格式化输出函数 */
void printk(const char *format, ...) {
   // 定义一个变量，用于存储可变参数
   va_list args;
   // 初始化可变参数
   va_start(args, format);
   // 定义一个字符数组，用于存储格式化后的字符串
   char buf[1024] = {0};
   // 将可变参数格式化成字符串，并存储到buf中
   vsprintf(buf, format, args);
   // 结束可变参数的获取
   va_end(args);
   // 将格式化后的字符串打印到控制台上
   console_put_str(buf);
}