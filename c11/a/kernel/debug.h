#ifndef _KERNEL_DEBUG_H
#define _KERNEL_DEBUG_H
void panic_spin(char *filename, int line, const char *func, const char *condition);
/*************************** __VA_ARGS__ ******************************* 
* __VA_ARGS__ 是预处理器所支持的专用标识符。
* 代表所有与省略号相对应的参数。
* "..."表示定义的宏其参数可变。*/ 

#define PANIC(...) panic_spin (__FILE__, __LINE__, __func__, __VA_ARGS__)  // LINE当前行号，func当前函数名 
/***********************************************************************/ 
#ifndef NDEBUG
#define ASSERT(CONDITION) ((void)0)
#else
#define ASSERT(CONDITION) \
    if (CONDITION){}      \
    else                  \
    {PANIC(#CONDITION);}
/*符号#让编译器将宏参数转化为字符串字面量*/
/*断言是if*/
#endif /* NDEBUG */ 
#endif /* _KERNEL_DEBUG_H */