section .data
    array db 1, 2, 3, 4, 5    ; 数组定义

section .text
global _start

_start:
    xor bx, bx                ; 清零 BX，作为索引
    mov cx, 5                 ; 数组长度

loop_start:
    mov al, [array + bx]      ; 读取数组元素到 AL
    ; 这里可以对 AL 进行操作
    inc bx                    ; 增加索引 BX
    loop loop_start           ; 循环直到 CX 为 0

    ; 结束程序
    mov eax, 1                ; 系统调用号：退出
    xor ebx, ebx              ; 退出码 0
    int 0x80                  ; 调用内核