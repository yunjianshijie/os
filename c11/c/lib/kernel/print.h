#ifndef _LIB_KERNEL_PRINT_H     
#define _LIB_KERNEL_PRINT_H    
#include "stdint.h"
#include "global.h"
void put_char(uint8_t char_asci);
void put_str(char *message);
void put_int(uint32_t num); //以16进制打印

#endif
