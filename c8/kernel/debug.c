#include "debug.h"
#include "interrupt.h"
#include "print.h"

/*打印文件名，行号，函数名，条件，并使函数悬停*/

void panic_spin(char *filename, int line, const char *func,
                const char *condition) {
  intr_disable(); // 先关掉中断
  put_str("\n\n\nerror!!!!!\n");
  put_str("\n filename:");
  put_str((char *)filename);
  put_str("\n line:");
  put_int(line);
  put_str("\n func:");
  put_str((char *)func);
  put_str("\n condition:");
  put_str((char *)condition);
  while (1)
    ;
}


