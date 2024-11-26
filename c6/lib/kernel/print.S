; （1）备份寄存器现场。
; （2）获取光标坐标值，光标坐标值是下一个可打印字符的位置。
; （3）获取待打印的字符。
; （4）判断字符是否为控制字符，若是回车符、换行符、退格符三种控制字符之一，则进入相应的处理流程。否则，其余字符都被粗暴地认为是可见字符，进入输出流程处理。
; （5）判断是否需要滚屏。
; （6）更新光标坐标值，使其指向下一个打印字符的位置。
; （7）恢复寄存器现场，退出。

TI_GDT equ 0  ;如果这个量为1就是使用LDT（局部），否则使用GDT（全局描述符表）就是TI位
RPL0 equ 0  ;RPL位请求特权级0
SELECTOR_VIDEO equ (0x0003 << 3) + RPL0 +TI_GDT ; 这一行计算视频段选择符。0x0003 是视频段的描述符索引（在 GDT 中的索引），通常用于指向 VGA 文本模式的段。

section .data
put_int_buffer dq 0                                         ; 定义8字节缓冲区用于数字到字符的转换

;定义选择子
[bits 32]
section .text
;------------------------ put_char ----------------------------- 
;功能描述：把栈中的 1 个字符写入光标所在处
;------------------------------------------------------------------- 
global put_char ; 声明全局函数
put_char:
;put_char 函数是以后我们任何一个打印功能的核心，所以光它的实现就要 112 行，这似乎是我们目前写过的最长的一个函数了，我保证以后也没有这么长的啦。
    pushad           ; 备份 32 位寄存器环境
    ;需要保证 gs 中为正确的视频段选择子
    ;为保险起见，每次打印时都为 gs 赋值
    ;压入所有双字长寄存器，EAX->ECX->EDX->EBX-> ESP-> EBP->ESI->EDI，EAX 是最先入栈。
    mov ax, SELECTOR_VIDEO ; 不能直接把立即数送入段寄存器
    mov gs, ax
;;获取当前光标位置 ;; 

    ;先获得最高8位
    ;要将索引 0x0e 写入 Address Register 寄存器，其端口为 0x03d4。
    mov dx ,0x03d4 ;索引寄存器，
    mov al ,0x0e ;光标位置高8位
    out dx ,al
    ;通过读写数据端口 0x3d5 来获得或设置光标位置
    mov dx ,0x03d5 ;数据寄存器
    in al ,dx  ;得到了光标位置的高8位
    mov ah ,al ;为什么不直接mov ah ,dx ，因为dx是16位寄存器，而ah是8位寄存器，所以需要把dx中的值存入al中，再存入ah中

    ;再获得最低8位
    mov dx ,0x03d4
    mov al ,0x0f
    out dx ,al
    mov dx ,0x03d5
    in al ,dx  ;得到了光标位置的最低8位

    ;高8位ah，低8位al，光标位置是ax

    ;把光标存入bx
    mov bx ,ax
    ;;获取待打印的字符 ;;
    mov ecx, [esp + 36]    ;pushad 压入 4×8＝32 字节，
                           ;加上主调函数 4 字节的返回地址，故 esp+36 字节
    cmp cl, 0xd            ;CR 是 0x0d，LF 是 0x0a
    jz .is_carriage_return
    cmp cl, 0xa
    jz .is_line_feed

    cmp cl, 0x8            ;BS asc是 0x08 
    jz .is_backspace
    jmp .put_other
    ;;;;;;;;;;;;;;45

    .is_backspace:
    ;;;;;;;;;;;;; backspace 的一点说明 ;;;;;;;;;; 
    ; ; 当为 backspace 时，本质上只要将光标移向前一个显存位置即可.后面再输入的字符自然会覆盖此处的字符
    ; 但有可能在键入backspace 后并不再键入新的字符，这时光标已经向前移动到待删除的字符位置，但字符还在原处
    ; 这就显得好怪异，所以此处添加了空格或空字符 0 
    dec bx
    shl bx, 1               ;光标左移 1 位等于乘 2 
                            ;表示光标对应显存中的偏移字节
    mov byte [gs:bx], 0x20  ;将待删除的字节补为 0 或空格皆可
    inc bx                  ;自减1后，bx 指向下一个字节，即属性字节
    mov byte [gs:bx], 0x07  ;属性字节为 0x07
    shr bx, 1               ;恢复 bx 为光标位置
    jmp .set_cursor
 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; 

.put_other:
    shl bx,1                ;光标位置用 2 字节表示，将光标值乘 2 
                            ;表示对应显存中的偏移字节
    mov [gs:bx],cl          ;ASCII字符本身
    inc bx                  ;bx 指向下一个字节，即属性字节
    mov byte [gs:bx],0x07     ;属性字节为 0x07
    shr bx,1                ;恢复 bx 为光标位置
    inc bx                  ;光标右移 1 位
    cmp bx,2000             ;判断是否需要滚屏
    jl .set_cursor          ;不需要滚屏，直接设置光标
    ;若超出屏幕字符数大小（2000）
    ; 显存的最后，则去设置新的光标值
    ; 若超出屏幕字符数大小（2000）
    ; 则换行处理
.is_line_feed: ;   换行符LF（\n）
.is_carriage_return: ; 回车符CR（\r）
 ;如果是 CR(\r)，只要把光标移到行首就行

xor dx ,dx ;清零dx
mov ax ,bx ;ax是被除数的低16位
mov si ,80      ;  由于是效仿 Linux，Linux 中\n 表示
                ;  下一行的行首，所以本系统中把\n 和\r 都处理为 Linux 中\n 的意思
div si          ;  除以80，商在ax中，余数在dx中;也就是下一行的行首 ;(ax% si)

sub bx ,dx      ;bx是光标位置，减去余数，就是下一行的行首 

.is_carriage_return_end:
    add bx, 80 ;如果是 LF(\n)，则光标下移一行
    cmp bx, 2000 
.is_line_feed_end:
    jl .set_cursor ;若没有超出屏幕字符数大小（2000），则设置光标

;;;86
;屏幕行范围是 0～24，滚屏的原理是将屏幕的第 1～24 行搬运到第 0～23 行
;再将第 24 行用空格填充

.roll_screen:
    cld
    mov ecx, 960 ; 24 行 × 80 字符 × 2 字节 = 1920 字节 2000-80=1920 个字符要搬运，共 1920*2=3840 字节
 ;一次搬 4 字节，共 3840/4=960 次
    mov esi,0xc00b80a0 ;显存地址
    mov edi,0xc00b8000 ;显存地址
    rep movsd ;重复执行，将显存中数据从esi搬移到edi，每次搬4字节

;;;;;将最后一排填充为空格
    mov ecx, 80 ; 最后一行有 80 个字符
    mov ebx ,3840 ; 最后一行的起始地址 1920* 2

.cls:
    mov word [gs:ebx], 0x0720 ; 0x07 是属性字节，0x20 是字符的 ASCII 码
    add ebx, 2 ; 每次搬移 2 字节
    loop .cls ; 循环 80 次
    mov bx,1920 ;将光标值重置


.set_cursor:
    ;;;;;;;;;;;;;;设置光标位置 
    ;;;;;;;;;; 1. 高8位 
    mov dx ,0x03d4 ;索引寄存器
    mov al ,0x0e ;光标位置高8位
    out dx ,al
    mov dx ,0x03d5 ;数据寄存器
    mov al ,bh ;光标位置高8位
    out dx ,al
    ;;;;;;;;;2. 低8位
    mov dx ,0x03d4
    mov al ,0x0f
    out dx ,al
    mov dx ,0x03d5
    mov al ,bl ;光标位置低8位
    out dx ,al
    ;;;;;;;;;;;;;;设置光标位置 
.put_char_done:
    popad
    ret

; 这里函数结束了

;--------------------------------------------
;put_str 通过put_char来打印以0字符结尾的字符串
;--------------------------------------------
;输入：栈中参数为打印的字符串
;输出：无
global put_str
put_str:
;
    push ebx
    push ecx
    xor ecx,ecx;
    mov ebx, [esp + 12]    ;pushad 压入 4×8＝32 字节，
.goon:
    mov cl ,[ebx]
    cmp cl ,0
    jz .str_over
    push ecx
    call put_char
    add esp,4
    inc ebx
    jmp .goon
.str_over:
    pop ecx
    pop ebx
    ret

;----------将小端字节序的数字变成对应的 ASCII 后，倒置---------- 
;输入:栈中参数为待打印的数字
;输出:在屏幕上打印十六进制数字，并不会打印前缀 0x 
;如打印十进制 15 时，只会直接打印 f，不会是 0xf 
;---------------------------------------------------------------------------------------

global put_int

put_int:
    pushad ;全部压入栈保存
    mov ebp,esp ;保存栈顶指针
    mov eax,[ebp+36] ;取出待打印的数字 36是call返回的地址（占4个）和8个pushad的偏移4字节
    mov edx,eax ;将待打印的数字存入 edx
    mov edi,7;指定在put_int_buffer中初始的偏移量
    mov ecx,8 ;指定循环次数，每次循环处理一个字节 。32 位数字中，十六进制数字的位数是 8 个
    mov ebx,put_int_buffer ;指向缓冲区
    
;将32位数字按照16进制的形式从低位到高位存一个一个处理
;共处理8个16进制数字

.16based_4bits:
    and edx, 0x0000000f ;解析16进制数字的每一位
                         ;and 与操作后，edx只有低4位有效
    cmp edx, 9          ;数字0～9和a～f需要分别处理成对应字符
    jg .is_A2F          ;
    add edx,'0'       ;数字0～9，加上字符'0'的ASCII码   
    jmp .store
.is_A2F:
    sub edx,10
    add edx,'A'       ;数字10～15，加上字符'A'的ASCII码

; 将每一位数字转换成对应的字符后，按照大端的属性
; 存入缓冲区 :put_int_buffer
;高位字符当到低地址，地位字符要放到高地址，这样的大段字符节序
;类似，只不过咱们这里是字符须4（小）
.store:
 ; 此时 dl 中应该是数字对应的字符的 ASCII 码
    mov [ebx+edi],dl ;将字符存入缓冲区
    dec edi ;缓冲区中下一个字符的位置
    shr eax ,4
    mov edx,eax
    loop .16based_4bits

;现在put_int_buffer中存的是大端字节序的字符，打印之前
;高位连续的字符去掉，不如把000123变成123

.ready_to_print:
    inc edi ;跳过缓冲区中高位连续的字符
.skip_prefix_0:
    cmp  edi,8          ;如果已经比较第9个字符了
                        ;表示待打印的字符串为全0

    je .full0
    ;找出连续的0字符，edi作为非0的最高字符


