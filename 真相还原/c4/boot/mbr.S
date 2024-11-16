; ;主引导程序 第二版通过显卡实现
; ; 
; ;LOADER_BASE_ADDR equ 0xA000 
; ;LOADER_START_SECTOR equ 0x2 
; ;------------------------------------------------------------
; section mbr vstart=0x7c00
; mov ax ,cs
; mov ds,ax
; mov es,ax
; mov ss,ax
; mov fs,ax
; mov sp,0x7c00
; mov ax,0xb800
; mov gs,ax 
; ;清屏
; ;利用 0x06 号功能，上卷全部行，则可清屏
; ; ----------------------------------------------------------- 
; ;INT 0x10 功能号：0x06 功能描述：上卷窗口
; ;------------------------------------------------------ 
; ;输入：
; ;AH 功能号= 0x06 
; ;AL = 上卷的行数（如果为 0，表示全部）
; ;BH = 上卷行属性
; ;(CL,CH) = 窗口左上角的(X,Y)位置
; ;(DL,DH) = 窗口右下角的(X,Y)位置
; ;无返回值: 
; mov ax,0600h
; mov bx,0700h
; mov cx,0            ; 左上角: (0, 0)
; mov dx,184fh        ; 右下角: (80,25)
;                     ; VGA 文本模式中，一行只能容纳 80 个字符，共 25 行
;                     ; 下标从 0 开始，所以 0x18=24，0x4f=79
; INT 10h             ;
; ; 输出背景色绿色，前景色红色，并且跳动的字符串"1 MBR"

; mov byte [gs:0x00],'1' 
; mov byte [gs:0x01],0xA4 ; A 表示绿色背景闪烁，4 表示前景色为红色
 
; mov byte [gs:0x02],' ' 
; mov byte [gs:0x03],0xA4 

; mov byte [gs:0x04],'M' 
; mov byte [gs:0x05],0xA4 

; mov byte [gs:0x06],'B' 
; mov byte [gs:0x07],0xA4 

; mov byte [gs:0x08],'R' 
; mov byte [gs:0x09],0xA4

; jmp $
; times 510-($-$$) db 0
; db 0x55, 0xaa


;主引导程序
;------------------------------------------------------------ 
%include "boot.inc" 
section mbr vstart=0x7c00
    mov ax,cs
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov fs,ax
    mov sp,0x7c00
    mov ax,0xb800
    mov gs,ax
     
 ;清屏
 ;利用 0x06 号功能，上卷全部行，则可清屏
 ; ----------------------------------------------------------- 
 ;INT 0x10 功能号：0x06 功能描述：上卷窗口
 ;------------------------------------------------------ 
 ;输入：
 ;AH 功能号= 0x06 
 ;AL = 上卷的行数（如果为 0，表示全部）
 ;BH = 上卷行属性
 ;(CL,CH) = 窗口左上角的(X,Y)位置
 ;(DL,DH) = 窗口右下角的(X,Y)位置
 ;无返回值：
    mov ax,0600h
    mov bx,0700h
    mov cx,0        ;左上角: (0, 0) 
    mov dx,184fh    ; 右下角: (80,25), 
; 因为 VGA 文本模式中，一行只能容纳 80 个字符，共 25 行
; 下标从 0 开始，所以 0x18=24，0x4f=79
    int 10h

    ;输出字符串
    mov byte [gs:0x00],'1' 
    mov byte [gs:0x01],0xA4 
 
    mov byte [gs:0x02],' ' 
    mov byte [gs:0x03],0xA4 

    mov byte [gs:0x04],'M' 
    mov byte [gs:0x05],0xA4
    ;A 表示绿色背景闪烁,4 表示前景色为红色
    mov byte [gs:0x06],'B' 
    mov byte [gs:0x07],0xA4 
 
    mov byte [gs:0x08],'R' 
    mov byte [gs:0x09],0xA4 

    mov eax,LOADER_START_SECTOR ;起始扇区lba地址
    mov bx,LOADER_BASE_ADDR     ;写入的地址
    mov cx,4                    ;带写入的扇区数,直接写第4个扇区
    call rd_disk_m_16           ;以下读取程序的起始部分

    jmp LOADER_BASE_ADDR
;------------------------------------------------------------------------------- 
;功能:读取硬盘 n 个扇区
rd_disk_m_16: 
;------------------------------------------------------------------------------- 
; eax=LBA 扇区号
; bx=将数据写入的内存地址
; cx=读入的扇区数
    mov esi,eax ;备份eax
    mov di,cx   ;备份cx
;读写硬盘
;第一步：设置要读取的扇区数
    mov dx,0x1f2
    mov al,cl
    out dx,al   ;读取的扇区数

    mov eax,esi ;恢复ax

;第 2 步:将 LBA 地址存入 0x1f3 ～ 0x1f6

    ;LBA 地址 7～0 位写入端口 0x1f3 
    mov dx,0x1f3
    out dx,al
    
    ;LBA 地址 15～8 位写入端口 0x1f4
    mov cl,8
    shr eax,cl
    mov dx,0x1f4
    out dx,al
    
    ;LBA 地址 23～16 位写入端口 0x1f5
    shr eax,cl
    mov dx,0x1f5
    out dx,al
    
    shr eax,cl
    and al,0x0f ;lba第24~27位
    or al,0xe0  ;设置7~4位为1110  ，表示lba模式
    mov dx,0x1f6 
    out dx,al

;第3步 向0xlf7端口写入读写命令0x20
    mov dx,0x1f7
    mov al,0x20
    out dx,al

;第四步，检查硬盘状态
.not_ready:
    ;同一端口，写是表示写入命令字，
nop 
in al,dx 
and al,0x88 ;第 4 位为 1 表示硬盘控制器已准备好数据传输
 ;第 7 位为 1 表示硬盘忙
cmp al,0x08 
jnz .not_ready ;若未准备好,继续等

;第 5 步:从 0x1f0 端口读数据
mov ax, di 
mov dx, 256 
mul dx 
mov cx, ax 
; di 为要读取的扇区数,一个扇区有 512 字节,每次读入一个字
; 共需 di*512/2 次,所以 di*256 
mov dx, 0x1f0 
.go_on_read: 
in ax,dx 
mov [bx],ax 
add bx,2 
loop .go_on_read 
ret 

times 510-($-$$) db 0 
db 0x55,0xaa


; %include "boot.inc"
; SECTION MBR vstart=0x7c00 ;起始地址编译为0x7c00
;     mov ax,cs     ; 因为是jmp 0:0x7c00跳转到MBR的，故cs此时为0。ds、es、ss、fs等sreg只能用通用寄存器赋值，本例采用ax赋值
;     mov ds,ax
;     mov es,ax
;     mov ss,ax
;     mov fs,ax
;     mov sp,0x7c00 ; 初始化栈指针
;     mov ax,0xb800 ; 0xb800为文本显示起始区
;     mov gs,ax     ; gs = ax 充当段基址的作用

;     ;ah = 0x06,al = 0x00 想要调用int 0x06的BIOS提供的中断对应的函数，即向上移动即完成清屏功能
;     ;cx,dx 分别存储左上角与右下角的左边，详情看int 0x06函数调用
;     mov ax,0x600
;     mov bx,0x700
;     mov cx,0
;     mov dx,0x184f

;     ;调用BIOS中断，实现清屏
;     int 0x10

;     ;新增功能：直接操作显存部分
;     ;预设输出"Hell0er."

;     mov byte [gs:0x00],'H'     ;低位字节储存ASCII字符，小端储存内存顺序相反。用关键词byte指定操作数所占空间，因为[gs:0x00]和'H'所占空间均为不定的，所以需要自己指定空>间大小
;     mov byte [gs:0x01],0xA4    ;背景储存在第二个字节，含字符与背景属性。A表示绿色背景闪烁，4表示前景色为红色

;     mov byte [gs:0x02],'e'
;     mov byte [gs:0x03],0xA4

;     mov byte [gs:0x04],'l'
;     mov byte [gs:0x05],0xA4

;     mov byte [gs:0x06],'l'
;     mov byte [gs:0x07],0xA4

;     mov byte [gs:0x08],'0'
;     mov byte [gs:0x09],0xA4

;     mov byte [gs:0x0A],'e'
;     mov byte [gs:0x0B],0xA4

;     mov byte [gs:0x0C],'r'
;     mov byte [gs:0x0D],0xA4

;     mov byte [gs:0x0E],'.'
;     mov byte [gs:0x0F],0xA4

;     mov eax,LOADER_START_SECTOR   ; 起始扇区lba地址
;     mov bx,LOADER_BASE_ADDR        ; 写入的地址
;     mov cx,4                       ; 待读入的扇区数
;     call rd_disk_m_16              ; 以下读取程序的起始部分（一个扇区）

;     jmp LOADER_BASE_ADDR

; ;-----------------------------------
; ;功能：读取硬盘n个扇区
; rd_disk_m_16:
; ;-----------------------------------
;                                    ; eax=LBA 扇区号
;                                    ; bx=将数据写入的内存地址
;                                    ; cx=读入的扇区数
;     mov esi,eax   ;备份eax
;     mov di,cx     ;备份cx
; ;读写硬盘：
; ;第1步：设置要读取的扇区数
;     mov dx,0x1f2
;     mov al,cl
;     out dx,al     ;读取的扇区数
;     mov eax,esi   ;恢复ax

; ;第2步：将LBA地址存入0x1f3~0x1f6
;     ;LBA地址7~0位写入端口0x1f3
;     mov dx,0x1f3
;     out dx,al

;     ;LBA地址15~8位写入端口0x1f4
;     mov cl,8
;     shr eax,cl
;     mov dx,0x1f4
;     out dx,al

;     ;LBA地址23~16位写入端口0x1f5
;     shr eax,cl
;     mov dx,0x1f5
;     out dx,al

;     shr eax,cl
;     and al,0x0f   ;lba第24~27位
;     or al,0xe0    ;设置7~4位为1110，表示lba模式
;     mov dx,0x1f6
;     out dx,al

; ;第3步：向0x1f7端口写入读命令,0x20
;     mov dx,0x1f7
;     mov al,0x20
;     out dx,al

; ;第4步：检测硬盘状态
;   .not_ready:
;     ;同一端口，写时表示写入命令字，读时表示读入硬盘状态
;     nop
;     in al,dx
;     and al,0x88   ;第4位为1表示硬盘控制器已准备好数据传输，第7位为1表示硬盘忙
;     cmp al,0x08
;     jnz .not_ready ;若未准备好，继续等

; ;第5步：从0x1f0端口读数据
;     mov ax,di
;     mov dx,256
;     mul dx
;     mov cx,ax    ;di为要读取的扇区数
;     mov dx,0x1f0
;   .go_on_read:
;     in ax,dx
;     mov [bx],ax
;     add bx,2
;     loop .go_on_read
;     ret

;     times 510-($-$$) db 0   ; 将512B的剩余部分填充为0
;     db 0x55,0xaa   ; 魔数

