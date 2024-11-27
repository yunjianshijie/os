// #include "print.h"
// int main(void) {
//   while (1)
//     ;
// }
#include "print.h"
void main(void) {
  put_str("I am kernel\n");
  put_char('k');
  put_char('e');
  put_char('r');
  put_char('n');
  put_char('e');
  put_char('l');
  put_char('\n');
  put_char('1');
  put_char('2');
  put_char('\b');
  put_char('4');
  put_char('\n');
  put_int(9);
  put_int(0x12345678);
  put_char('\n');
  while (1)
    ;
  }