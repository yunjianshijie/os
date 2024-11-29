[ bits 32]
%define ERROR_CODE nop
%define ZERO push 0
extern pet_str; 声明外部函数

section .data
intr_str db "Interrupt occur!", 0xa,0
global intr_entry_table
intr_entry_table:

%macro VECTOR 2
section .text
intr%1entry:                     ;每个中断处理程序都要压入中断向量号
                                ;所以一个中断类型一个中断处理程序
                                ;自己知道自己的中断向量号是多少
    %2
    push intr_str
    call pet_str
    add esp,4               ;跳过参数

    ;如果是从片上进入中断，除了往从片上发EOI外，还要往主片上发EOI

    mov al ,0x20            ;中断结束命令 EOI
    out 0xa0 , al           ;向从片发送
    out 0x20 , al           ;向主片发送
    add esp, 4              ;跨过error_code()
    iret                    ;从中断返回，32位下等同指令iretd
section .data
    dd intr%1entry          ;储存各个中断入口程序的地址
                            ;形成intr_entry_table数组
%endmacro
VECTOR 0x00, ZERO
VECTOR 0x01, ZERO
VECTOR 0x02, ZERO
VECTOR 0x03, ZERO
VECTOR 0x04, ZERO
VECTOR 0x05, ZERO
VECTOR 0x06, ERROR_CODE
VECTOR 0x07, ZERO
VECTOR 0x08, ZERO
VECTOR 0x09, ZERO
VECTOR 0x0a, ZERO
VECTOR 0x0b, ZERO
VECTOR 0x0c, ZERO
VECTOR 0x0d, ZERO
VECTOR 0x0e, ERROR_CODE
VECTOR 0x0f, ZERO
VECTOR 0x10, ZERO
VECTOR 0x11, ZERO
VECTOR 0x12, ZERO
VECTOR 0x13, ZERO
VECTOR 0x14, ZERO
VECTOR 0x15, ZERO
VECTOR 0x16, ZERO
VECTOR 0x17, ZERO
VECTOR 0x18, ZERO
VECTOR 0x19, ZERO
VECTOR 0x1a, ZERO
VECTOR 0x1b, ZERO
VECTOR 0x1c, ZERO
VECTOR 0x1d, ZERO
VECTOR 0x1e, ERROR_CODE 
VECTOR 0x1f, ZERO 
VECTOR 0x20, ZERO 



