;;
 %include "boot.inc"
section loader vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR
; ;输出背景绿色，前景色红色，并且跳动的字符串‘1 MBR’
jmp loader_start;loader一进来是一大堆GDT段描述符数据，无法执行，所以要跳过
;构建GDT和它内部描述符
GDT_BASE: ; GDT的起始地址
    dd 0x00000000
    dd 0x00000000
CODE_DESC: ; Code Segment Descriptor 代码段描述符
    dd 0x0000FFFF ; Limit(段界限) 0-15位
    dd DESC_CODE_HIGH4
DATA_STACK_DESC:
    dd 0x0000FFFF ; Limit(段界限) 0-15位
    dd DESC_DATA_HIGH4
VIDEO_DESC:
    dd 0x80000007
    dd DESC_VIDEO_HIGH4

    GDT_SIZE equ $ - GDT_BASE
    GDT_LIMIT equ GDT_SIZE - 1
    ;times 60 dq 0 ;填充60个8字节，用于填充GDT表，防止溢出
    times 59 dq 0
    times 5 db 0
    total_mem_bytes dd 0
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    SELECTOR_CODE equ (0x0001<< 3) +TI_GDT+RPL0
     ;相当于(CODE_DESC-GDT_BASE)/8+TI_GDT+RPL0
    SELECTOR_DATA equ (0x0002<< 3) +TI_GDT+RPL0
    SELECTOR_VIDEO equ (0x0003<< 3) +TI_GDT+RPL0
   
    ; total_mem_bytes 用于保存内存容量，以字节为单位，此位置比较好记
    ;当前偏移 loader.bin 文件头 0x200 字节
    ; loader.bin 的加载地址是 0x900 
    ; 故 total_mem_bytes 内存中的地址是 0xb00 
    ; 将来在内核中咱们会引用此地址
    
    gdt_ptr: dw GDT_LIMIT; GDT 在内存中的偏移地
        dd GDT_BASE

    ;人工对齐:total_mem_bytes4+gdt_ptr6+ards_buf244+ards_nr2，共 256 字节
    ards_buf times 244 db 0 
    ards_nr dw 0 ;用于记录 ARDS 结构体数量
loader_start:
   ;int 15h eax=0000E820h, edx=534D4150h('SMAP'的ASCII码)获取内存布局\
    
    xor ebx,ebx                    ; 第一次调用时，ebx 必须为 0

    mov edx,0x534d4150             ; edx只赋值一次,循环体中不会改变
    mov di ,ards_buf               ; ards 结构缓解区
.e820_mem_get_loop:                ; 循环获取ARDS内存范围描述结构体
    mov eax,0x0000e820                  ; eax = 0xe820 执行 int 0x15 后，eax 值变为 0x534d4150
;所以每次执行  int dou都要更新子功能号
    mov ecx,20                      ; ecx = ARDS 结构体大小
    int 0x15                        ; 调用 int 0x15
    jc .e820_failed_so_try_e801          ; 如果 CF=1，表示调用失败
    mov cx,20
    add di ,cx                     ; ARDS 结构体大小为 20 字节
    inc word [ards_nr]              ; ards_nr 记录 ARDS 结构体数量
    cmp ebx ,0                      ; 判断是否还有其他内存范围描述结构体
    jnz .e820_mem_get_loop          ; 如果 ebx 不为 0，表示还有其他内存范围描述结构体，继续循环获取
    ;在所有 ards 结构中
 ;找出(base_add_low + length_low)的最大值，即内存的容量
    mov cx , [ards_nr]              ; ards_nr 记录 ARDS 结构体数量
    mov ebx ,ards_buf               ; ards 结构缓冲区
    xor eax ,eax                    ; 清空 eax
    xor edx ,edx                    ; 清空 edx
    .find_max_mem_area:
    mov eax ,[ebx]
    add eax ,[ebx + 8]
    add ebx,20
    cmp edx ,eax
;冒泡排序,找出最大,edx 寄存器始终是最大的内存容量
    jge .next_ards
    mov edx,eax                 ;edx为总内存大小
.next_ards:
    loop .find_max_mem_area
    jmp .mem_get_ok


 ;------ int 15h ax = E801h 获取内存大小，最大支持 4G ------
 ;返回后,ax cx 值一样,以kb为单位位，bx dx 值一样，以 64KB 为单位
 ; 在ax和cx中寄存器中低16MB,在bx和dx中寄存器中16MB到4G
 .e820_failed_so_try_e801:
    mov eax ,0x0000e801
    int 0x15
    jc .e801_failed_so_try88 ;若当前 e801 方法失败，就尝试 0x88 方法

;1 先算出低15MB内存
; ax和cx中是以kb为单位的内存数量，单位将其转换为以 byte 为单位
    mov cx,0x400
    mul cx
    shl edx, 16
    and eax,0x0000ffff
    or edx,eax
    add edx, 0x100000   ; ax只是15mb,所以要加1mb
    mov esi ,edx        ; 先把低15mb

;2 再算出高16MB内存
; bx和dx中是以64KB为单位的内存数量，单位将其转换为以 byte 为单位
xor eax,eax
    mov ax,bx
    mov ecx,0x10000     ;0x10000 十进制为 64KB 
    mul ecx             ;32 位乘法，默认的被乘数是 eax，积为 64 位
 
    add esi,edx        ;把高16MB加到低15MB上
    mov edx,esi        ;edx寄存器中保存了内存大小
    jmp .mem_get_ok

;------ int 0x88 获取内存大小，最大支持 64MB ------
.e801_failed_so_try88: 
    mov ah,0x88
    int 0x15
    jc .error_hlt
    and eax,0x0000ffff
    ;16 位乘法，被乘数是 ax，积为 32 位。积的高 16 位在 dx 中
;积的低 16 位在 ax 中
    mov cx ,0x400
    ;0x400 十进制为 1024将ax的内存容量换位以byte 为单位
    mul cx
    shl edx,16          ;把dx移到16位
    or edx,eax          ;把ax加到dx上
    add edx,0x100000     ;;0x88 子功能只会返回 1MB 以上的内存
            ;故实际内存大小要加上 1MB 
   
.error_hlt:
    jmp $
.mem_get_ok:
    mov [total_mem_bytes],edx
    
;     mov sp,LOADER_BASE_ADDR
;     mov	bp,loadermsg                                    ; ES:BP = 字符串地址
;     mov	cx,17			                                ; CX = 字符串长度
;     mov	ax,0x1301		                                ; AH = 13,  AL = 01h
;     mov	bx,0x001f		                                ; 页号为0(BH = 0) 蓝底粉红字(BL = 1fh)
;     mov	dx,0x1800		                                ;
;     int	0x10       
;-----------------   准备进入保护模式   ------------------------------------------
;1 打开A20
;2 加载gdt
;3 将cr0的pe位置1
 
;-----------------  打开A20  ----------------
in al, 0x92
or al, 0000_0010B
out 0x92,al
;-----------------  加载GDT  ----------------
lgdt [gdt_ptr] ; load GDT [addr]
;-----------------  cr0第0位置1  ----------------
mov eax,cr0
or eax,0x00000001
mov cr0,eax
jmp dword SELECTOR_CODE:p_mode_start

[bits 32]

p_mode_start:
    mov ax,SELECTOR_DATA
    mov ds,ax
    mov es,ax
    mov fs,ax
    mov ss,ax
    mov esp,LOADER_STACK_TOP
    mov ax,SELECTOR_VIDEO
    mov gs,ax
    mov byte [gs:160],'P'

    jmp $


