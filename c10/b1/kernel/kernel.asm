; [ bits 32]
; %define ERROR_CODE nop
; %define ZERO push 0
; extern put_str; 声明外部函数

; section .data
; intr_str db "Interrupt occur!", 0xa,0
; global intr_entry_table
; intr_entry_table:

; %macro VECTOR 2
; section .text
; intr%1entry:                    ;每个中断处理程序都要压入中断向量号
;                                 ;所以一个中断类型一个中断处理程序
;                                 ;自己知道自己的中断向量号是多少
;     %2
; ;保存上下文环境
;     push ds                 ;以下是保存上下文环境
;     push es
;     push fs
;     push gs
;     pushad                  ;PUSHAD 指令压入 32 位寄存器，其入栈顺序是：
; ; EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI,EAX 最先入栈

;  ;如果是从片上进入的中断
;  ;除了往从片上发送 EOI 外，还要往主片上发送 EOI
;     mov al,0x20     ; 中断结束命令 EOI
;     out 0xa0,al     ; 向从片发送
;     out 0x20,al     ; 向向主片发送

;     push %1                 ;不管idt_table中的目标程序是否需要参数
;                             ; 都一律压入中断向量号，测试时很方便了

;     call [idt_table + %1*4 ]    ; 调用 idt_table 中的 C 版本中断处理函数
;     jmp intr_exit               ; 调用idt_table中的C版本中断处理函数后

;     ; push intr_str
;     ; call put_str
;     ; add esp,4               ;跳过参数

;     ; ;如果是从片上进入中断，除了往从片上发EOI外，还要往主片上发EOI

;     ; mov al ,0x20            ;中断结束命令 EOI
;     ; out 0xa0 , al           ;向从片发送
;     ; out 0x20 , al           ;向主片发送
;     ; add esp, 4              ;跨过error_code()
;     ; iret                    ;从中断返回，32位下等同指令iretd

; section .data
;     dd intr%1entry          ;储存各个中断入口程序的地址
;                             ;形成intr_entry_table数组
; %endmacro

; section .text
; global intr_exit
; intr_exit:

;     add esp, 4 ;跳过(中断号)
;     popad 
;     pop gs
;     pop fs
;     pop es
;     pop ds
;     add esp,4   ;跨过error_code()
;     iretd

; VECTOR 0x00,ZERO                            ;调用之前写好的宏来批量生成中断处理函数，传入参数是中断号码与上面中断宏的%2步骤，这个步骤是什么都不做，还是压入0看p303
; VECTOR 0x01,ZERO
; VECTOR 0x02,ZERO
; VECTOR 0x03,ZERO
; VECTOR 0x04,ZERO
; VECTOR 0x05,ZERO
; VECTOR 0x06,ZERO
; VECTOR 0x07,ZERO
; VECTOR 0x08,ERROR_CODE
; VECTOR 0x09,ZERO
; VECTOR 0x0a,ERROR_CODE
; VECTOR 0x0b,ERROR_CODE
; VECTOR 0x0c,ZERO
; VECTOR 0x0d,ERROR_CODE
; VECTOR 0x0e,ERROR_CODE
; VECTOR 0x0f,ZERO
; VECTOR 0x10,ZERO
; VECTOR 0x11,ERROR_CODE
; VECTOR 0x12,ZERO
; VECTOR 0x13,ZERO
; VECTOR 0x14,ZERO
; VECTOR 0x15,ZERO
; VECTOR 0x16,ZERO
; VECTOR 0x17,ZERO
; VECTOR 0x18,ERROR_CODE
; VECTOR 0x19,ZERO
; VECTOR 0x1a,ERROR_CODE
; VECTOR 0x1b,ERROR_CODE
; VECTOR 0x1c,ZERO
; VECTOR 0x1d,ERROR_CODE
; VECTOR 0x1e,ERROR_CODE
; VECTOR 0x1f,ZERO
; VECTOR 0x20,ZERO


[bits 32]
%define ERROR_CODE nop                      ;有些中断进入前CPU会自动压入错误码32位,为保持栈中格式统一,这里不做操作.
%define ZERO push 0                         ;有些中断进入前CPU不会压入错误码,对于这类中断,我们为了与前一类中断统一管理,就自己压入32位的0

extern idt_table                            ;idt_table是C中注册的中断处理程序数组

section .data
;intr_str db "interrupt occur!", 0xa, 0      ;第二个是一个换行符，第三个定义一个ascii码为0的字符，用来表示字符串的结尾
global intr_entry_table
intr_entry_table:                           ;编译器会将之后所有同属性的section合成一个大的segment，所以这个标号后面会聚集所有的中断处理程序的地址

%macro VECTOR 2                             ;汇编中的宏用法见书p320
section .text                               ;中断处理函数的代码段
intr%1entry:                                ;每个中断处理程序都要压入中断向量号,所以一个中断类型一个中断处理程序，自己知道自己的中断向量号是多少,此标号来表示中断处理程序的入口
    %2                                      ;这一步是根据宏传入参数的变化而变化的
    push ds                                 ;以下是保存上下文环境
    push es
    push fs
    push gs
    pushad
                                            ;如果是从片上进入的中断,除了往从片上发送EOI外,还要往主片上发送EOI 
    mov al,0x20                             ;中断结束命令EOI
    out 0xa0,al                             ;向主片发送OCW2,其中EOI位为1，告知结束中断，详见书p317
    out 0x20,al                             ;向从片发送OCW2,其中EOI位为1，告知结束中断

    push %1                                 ;不管idt_table中的目标程序是否需要参数,都一律压入中断向量号,调试时很方便
    call [idt_table + %1*4]                 ;调用idt_table中的C版本中断处理函数
    jmp intr_exit

section .data                               ;这个段就是存的此中断处理函数的地址
    dd    intr%1entry                       ;存储各个中断入口程序的地址，形成intr_entry_table数组,定义的地址是4字节，32位
%endmacro

section .text
global intr_exit
intr_exit:
                                            ;以下是恢复上下文环境
    add esp, 4                              ;跳过中断号
    popad
    pop gs
    pop fs
    pop es
    pop ds
    add esp, 4                              ;对于会压入错误码的中断会抛弃错误码（这个错误码是执行中断处理函数之前CPU自动压入的），对于不会压入错误码的中断，就会抛弃上面push的0
    iretd                                   ;从中断返回,32位下iret等同指令iretd


VECTOR 0x00,ZERO                            ;调用之前写好的宏来批量生成中断处理函数，传入参数是中断号码与上面中断宏的%2步骤，这个步骤是什么都不做，还是压入0看p303
VECTOR 0x01,ZERO
VECTOR 0x02,ZERO
VECTOR 0x03,ZERO
VECTOR 0x04,ZERO
VECTOR 0x05,ZERO
VECTOR 0x06,ZERO
VECTOR 0x07,ZERO
VECTOR 0x08,ERROR_CODE
VECTOR 0x09,ZERO
VECTOR 0x0a,ERROR_CODE
VECTOR 0x0b,ERROR_CODE
VECTOR 0x0c,ZERO
VECTOR 0x0d,ERROR_CODE
VECTOR 0x0e,ERROR_CODE
VECTOR 0x0f,ZERO
VECTOR 0x10,ZERO
VECTOR 0x11,ERROR_CODE
VECTOR 0x12,ZERO
VECTOR 0x13,ZERO
VECTOR 0x14,ZERO
VECTOR 0x15,ZERO
VECTOR 0x16,ZERO
VECTOR 0x17,ZERO
VECTOR 0x18,ERROR_CODE
VECTOR 0x19,ZERO
VECTOR 0x1a,ERROR_CODE
VECTOR 0x1b,ERROR_CODE
VECTOR 0x1c,ZERO
VECTOR 0x1d,ERROR_CODE
VECTOR 0x1e,ERROR_CODE
VECTOR 0x1f,ZERO
VECTOR 0x20,ZERO