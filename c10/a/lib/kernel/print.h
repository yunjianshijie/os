#ifndef _LIB_KERNEL_PRINT_H     
#define _LIB_KERNEL_PRINT_H    
#include "stdint.h"
void put_char(uint8_t char_asci);
void put_str(char *message);
void put_int(uint32_t num); //以16进制打印
void set_cursor(uint32_t x);
#define NULL ((void *)0)
#endif
