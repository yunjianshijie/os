section .data
str_c_lib: db "Hello, World!1", 0xa
str_c_lib_len equ $-str_c_lib

str_syscall: db "Hello, World!2", 0xa
str_syscall_len equ $-str_syscall

section .text
global _start

_start:
    ; C library version
    push str_c_lib_len
    push str_c_lib
    push 1

    call simu_write ;调用自定义的 simu_write 函数
    add esp, 12  ;回收栈空间

    mov ebx, 1
    mov eax,4 ;  用说号子功能号是write系统调用
    mov edx ,str_syscall_len
    mov ecx ,str_syscall
    int 0x80
    mov eax,1
    int 0x80

   ;;;;;;;下面自定义的 simu_write 用来模拟 C 库中系统调用函数 write 
 ;;;;;;这里模拟它的实现原理


simu_write:
    push ebp
    mov ebp,esp
    mov ebx,[ebp+8] ;ebx = fd
    mov ecx,[ebp+12] ;ecx = str
    mov edx,[ebp+16] ;edx = len
    mov eax,4 ;  用说号子功能号是write系统调用
    int 0x80
    pop ebp
    ret

;;;;;;; nasm -f elf32 -o syscall_write.o 1/syscall_write.S
;其中-f 参数用来指定编译输出的文件格式，这里需要指定为 elf，目的是将来要和 gcc 编译的 elf 格式d 目标文件链接，所以格式必须相同          

;;ld -m elf_i386 -o syscall_write.bin syscall_write.o ;
;;这里要指定-m 参数，用来指定链接的格式，这里需要指定为 elf_i386，目的是将来要和 gcc 编译的 elf 格式d 目标文件链接，所以格式必须相同


