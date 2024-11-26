// extern void asm_print(char *,int);

// void c_print(char *str) {
//     int len = 0;
//     while(str[len] != '\0'){
//         len++;
//     }
//     asm_print(str, len);
// }

//#include <gnu/stubs-32.h>
// #include <stdio.h>
extern void asm_print(char *str, int len);

void c_print(char *str) {
  int len = 0;
  while (str[len] != '\0') {
    len++;
  }
  asm_print(str, len);
}

// gcc - m32 - c 1 / C_with_S_c.c -o 1 / C_with_S_c.o

// #yunjian @yunjianWorld in ~ / os on git : main x[18 : 46 : 48]
//               $ nasm -f elf32 1 / C_with_S_S.S -o 1 / C_with_S_S.o

// #yunjian @yunjianWorld in ~ / os on git : main x[18 : 46 : 55]
//               $ gcc -m32 - o output_program 1 / C_with_S_c.o 1 / C_with_S_S.o



