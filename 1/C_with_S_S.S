section .data
str: db "Hello World!1",0xa,0 ;0xa是换行

str_len equ $-str

section .text
extern c_print 
global _asm_print

_asm_print:
    ;调用 c 代码中的函数 c_print 
    push str ;转入参数
    call c_print ;调用c函数
    add esp, 4 ;平衡栈
;; 退出程序 ;
    mov eax, 1
    int 0x80

global asm_print
asm_print:
    push ebp
    mov ebp, esp
    mov eax, 4
    mov ebx, 1
    mov ecx, [ebp+8]
    mov edx, [ebp+12]
    int 0x80
    pop ebp
    ret


