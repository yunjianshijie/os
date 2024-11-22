; %include "boot.inc"
; section loader vstart=LOADER_BASE_ADDR
; LOADER_STACK_TOP equ LOADER_BASE_ADDR
; ; ;输出背景绿色，前景色红色，并且跳动的字符串‘1 MBR’
; jmp loader_start;loader一进来是一大堆GDT段描述符数据，无法执行，所以要跳过
; ;构建GDT和它内部描述符
; GDT_BASE: ; GDT的起始地址
;     dd 0x00000000
;     dd 0x00000000
; CODE_DESC: ; Code Segment Descriptor 代码段描述符
;     dd 0x0000FFFF ; Limit(段界限) 0-15位
;     dd DESC_CODE_HIGH4
; DATA_STACK_DESC:
;     dd 0x0000FFFF ; Limit(段界限) 0-15位
;     dd DESC_DATA_HIGH4
; VIDEO_DESC:
;     dd 0x80000007
;     dd DESC_VIDEO_HIGH4

;     GDT_SIZE equ $ - GDT_BASE
;     GDT_LIMIT equ GDT_SIZE - 1
;     ;times 60 dq 0 ;填充60个8字节，用于填充GDT表，防止溢出
;     times 59 dq 0
;     times 5 db 0
;     total_mem_bytes dd 0
;     ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;     SELECTOR_CODE equ (0x0001<< 3) +TI_GDT+RPL0
;      ;相当于(CODE_DESC-GDT_BASE)/8+TI_GDT+RPL0
;     SELECTOR_DATA equ (0x0002<< 3) +TI_GDT+RPL0
;     SELECTOR_VIDEO equ (0x0003<< 3) +TI_GDT+RPL0
   
;     ; total_mem_bytes 用于保存内存容量，以字节为单位，此位置比较好记
;     ;当前偏移 loader.bin 文件头 0x200 字节
;     ; loader.bin 的加载地址是 0x900 
;     ; 故 total_mem_bytes 内存中的地址是 0xb00 
;     ; 将来在内核中咱们会引用此地址
    
;     gdt_ptr: dw GDT_LIMIT; GDT 在内存中的偏移地
;         dd GDT_BASE

;     ;人工对齐:total_mem_bytes4+gdt_ptr6+ards_buf244+ards_nr2，共 256 字节
;     ards_buf times 244 db 0 
;     ards_nr dw 0 ;用于记录 ARDS 结构体数量
; loader_start:
;    ;int 15h eax=0000E820h, edx=534D4150h('SMAP'的ASCII码)获取内存布局\
    
;     xor ebx,ebx                    ; 第一次调用时，ebx 必须为 0

;     mov edx,0x534d4150             ; edx只赋值一次,循环体中不会改变
;     mov di ,ards_buf               ; ards 结构缓解区
; .e820_mem_get_loop:                ; 循环获取ARDS内存范围描述结构体
;     mov eax,0x0000e820                  ; eax = 0xe820 执行 int 0x15 后，eax 值变为 0x534d4150
; ;所以每次执行  int dou都要更新子功能号
;     mov ecx,20                      ; ecx = ARDS 结构体大小
;     int 0x15                        ; 调用 int 0x15
;     jc .e820_failed_so_try_e801          ; 如果 CF=1，表示调用失败
;     mov cx,20
;     add di ,cx                     ; ARDS 结构体大小为 20 字节
;     inc word [ards_nr]              ; ards_nr 记录 ARDS 结构体数量
;     cmp ebx ,0                      ; 判断是否还有其他内存范围描述结构体
;     jnz .e820_mem_get_loop          ; 如果 ebx 不为 0，表示还有其他内存范围描述结构体，继续循环获取
;     ;在所有 ards 结构中
;  ;找出(base_add_low + length_low)的最大值，即内存的容量
;     mov cx , [ards_nr]              ; ards_nr 记录 ARDS 结构体数量
;     mov ebx ,ards_buf               ; ards 结构缓冲区
;     xor eax ,eax                    ; 清空 eax
;     xor edx ,edx                    ; 清空 edx
;     .find_max_mem_area:
;     mov eax ,[ebx]
;     add eax ,[ebx + 8]
;     add ebx,20
;     cmp edx ,eax
; ;冒泡排序,找出最大,edx 寄存器始终是最大的内存容量
;     jge .next_ards
;     mov edx,eax                 ;edx为总内存大小
; .next_ards:
;     loop .find_max_mem_area
;     jmp .mem_get_ok


;  ;------ int 15h ax = E801h 获取内存大小，最大支持 4G ------
;  ;返回后,ax cx 值一样,以kb为单位位，bx dx 值一样，以 64KB 为单位
;  ; 在ax和cx中寄存器中低16MB,在bx和dx中寄存器中16MB到4G
;  .e820_failed_so_try_e801:
;     mov eax ,0x0000e801
;     int 0x15
;     jc .e801_failed_so_try88 ;若当前 e801 方法失败，就尝试 0x88 方法

; ;1 先算出低15MB内存
; ; ax和cx中是以kb为单位的内存数量，单位将其转换为以 byte 为单位
;     mov cx,0x400
;     mul cx
;     shl edx, 16
;     and eax,0x0000ffff
;     or edx,eax
;     add edx, 0x100000   ; ax只是15mb,所以要加1mb
;     mov esi ,edx        ; 先把低15mb

; ;2 再算出高16MB内存
; ; bx和dx中是以64KB为单位的内存数量，单位将其转换为以 byte 为单位
; xor eax,eax
;     mov ax,bx
;     mov ecx,0x10000     ;0x10000 十进制为 64KB 
;     mul ecx             ;32 位乘法，默认的被乘数是 eax，积为 64 位
 
;     add esi,edx        ;把高16MB加到低15MB上
;     mov edx,esi        ;edx寄存器中保存了内存大小
;     jmp .mem_get_ok

; ;------ int 0x88 获取内存大小，最大支持 64MB ------
; .e801_failed_so_try88: 
;     mov ah,0x88
;     int 0x15
;     jc .error_hlt
;     and eax,0x0000ffff
;     ;16 位乘法，被乘数是 ax，积为 32 位。积的高 16 位在 dx 中
; ;积的低 16 位在 ax 中
;     mov cx ,0x400
;     ;0x400 十进制为 1024将ax的内存容量换位以byte 为单位
;     mul cx
;     shl edx,16          ;把dx移到16位
;     or edx,eax          ;把ax加到dx上
;     add edx,0x100000     ;;0x88 子功能只会返回 1MB 以上的内存
;             ;故实际内存大小要加上 1MB 
   
; .error_hlt:
;     jmp $
; .mem_get_ok:
;     mov [total_mem_bytes],edx
    
; ;     mov sp,LOADER_BASE_ADDR
; ;     mov	bp,loadermsg                                    ; ES:BP = 字符串地址
; ;     mov	cx,17			                                ; CX = 字符串长度
; ;     mov	ax,0x1301		                                ; AH = 13,  AL = 01h
; ;     mov	bx,0x001f		                                ; 页号为0(BH = 0) 蓝底粉红字(BL = 1fh)
; ;     mov	dx,0x1800		                                ;
; ;     int	0x10       
; ;-----------------   准备进入保护模式   ------------------------------------------
; ;1 打开A20
; ;2 加载gdt
; ;3 将cr0的pe位置1
 
; ;-----------------  打开A20  ----------------
; in al, 0x92
; or al, 0000_0010B
; out 0x92,al
; ;-----------------  加载GDT  ----------------
; lgdt [gdt_ptr] ; load GDT [addr]
; ;-----------------  cr0第0位置1  ----------------
; mov eax,cr0
; or eax,0x00000001
; mov cr0,eax
; jmp dword SELECTOR_CODE:p_mode_start

; [bits 32]
; p_mode_start:
;     mov ax,SELECTOR_DATA
;     mov ds,ax
;     mov es,ax
;     mov fs,ax
;     mov ss,ax
;     mov esp,LOADER_STACK_TOP
;     mov ax,SELECTOR_VIDEO
;     mov gs,ax
; ;------------------- 建立页表 -------------------
; setup_page:
;    mov ecx,4096
;    mov esi,0
; .clear_page_dir:
;    mov byte [PAGE_DIR_TABLE_POS+esi],0
;    inc esi
;    loop .clear_page_dir

;    ;创建页目录项(PDE)
; .create_pde:
;    mov eax,PAGE_DIR_TABLE_POS
;    add eax,0x1000               ;此时的eax为第一个页表的物理地址
;    mov ebx,eax                  ;ebx=eax，为后续的.create_pte做准备，ebx为基址

;    ;下面将偏移地址0x0（第1个）和0xc00（第768个页目录项）存为第1个页表的地址，每个页表表示4MB内存
;    or eax,PG_US_U|PG_RW_W|PG_P        ;最低特权级|可读写|存在
;    mov [PAGE_DIR_TABLE_POS+0x0],eax   ;第1个页目录项
;    mov [PAGE_DIR_TABLE_POS+0xc00],eax ;第768个页目录项
;    sub eax,0x1000
;    mov [PAGE_DIR_TABLE_POS+4092],eax  ;最后一个页目录项指向页目录自己

;    ;创建页表项(PTE)
;    mov ecx,256                  ;对低端内存1MB建页表：1MB/4KB=256（256个页表项，1个页表足矣）
;    mov esi,0
;    mov edx,PG_US_U|PG_RW_W|PG_P ;最低特权第|可读写|存在
; .create_pte:
;    mov [ebx+esi*4],edx          ;逐个页表项设置
;    add edx,4096                 ;因为1个页表4KB，所以edx的基址+4KB
;    inc esi
;    loop .create_pte

;    ;创建内核其它页表的PDE
;    mov eax,PAGE_DIR_TABLE_POS
;    add eax,0x2000               ;此时的eax为第二个页表的物理地址
;    or eax,PG_US_U|PG_RW_W|PG_P  ;最低特权级|可读写|存在
;    mov ebx,PAGE_DIR_TABLE_POS
;    mov ecx,254
;    mov esi,769
; .create_kernel_pde:
;    mov [ebx+esi*4],eax          ;将第2个~第256个页表的地址逐个存入页表项
;    inc esi
;    add eax,0x1000               ;下一个页表的地址
;    loop .create_kernel_pde
;    ret
; ;创建页目录及页表并初始化页内存位图
;    call setup_page
;    ;要将描述符表地址及偏移量写入内存gdt_ptr，一会儿用新地址重新加载
;    sgdt [gdt_ptr]              ;存储到原来gdt所在的位置

;    mov ebx,[gdt_ptr+2]
;    or dword [ebx+0x18+4],0xc0000000

;    add dword [gdt_ptr+2],0xc0000000
;    add esp,0xc0000000

;    mov eax,PAGE_DIR_TABLE_POS
;    mov cr3,eax
;    mov eax,cr0
;    or eax,0x80000000
;    mov cr0,eax

;    lgdt [gdt_ptr]

;    mov byte [gs:160],'V'

;    jmp $



%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR

;构建gdt及其内部的描述符
GDT_BASE:
    dd 0x00000000
	dd 0x00000000

CODE_DESC:
    dd 0x0000FFFF
	dd DESC_CODE_HIGH4

DATA_STACK_DESC:
    dd 0x0000FFFF
    dd DESC_DATA_HIGH4

VIDEO_DESC:
    dd 0x80000007
    dd DESC_VIDEO_HIGH4

    GDT_SIZE equ $ - GDT_BASE
    GDT_LIMIT equ GDT_SIZE - 1
    times 60 dq 0
    SELECTOR_CODE equ (0x0001<<3) + TI_GDT + RPL0
    SELECTOR_DATA equ (0x0002<<3) + TI_GDT + RPL0
    SELECTOR_VIDEO equ (0x0003<<3) + TI_GDT + RPL0

total_mem_bytes dd 0
gdt_ptr dw GDT_LIMIT
   dd  GDT_BASE

ards_buf times 244 db 0
ards_nr dw 0

loader_start:
    xor ebx, ebx
    mov edx, 0x534d4150
    mov di, ards_buf
.e820_mem_get_loop:
    mov eax, 0x0000e820
    mov ecx, 20
    int 0x15
    add di, cx
    inc word [ards_nr]
    cmp ebx, 0
    jnz .e820_mem_get_loop
    mov cx, [ards_nr]
    mov ebx, ards_buf
    xor edx, edx
.find_max_mem_area:
    mov eax, [ebx]
    add eax, [ebx+8]
    add ebx, 20
    cmp edx, eax
    jge .next_ards
    mov edx, eax
.next_ards:
    loop .find_max_mem_area
    mov [total_mem_bytes], edx
;-----------------   准备进入保护模式   ------------------------------------------
;-----------------  打开A20  ----------------
    in al, 0x92
    or al, 0000_0010B
    out 0x92,al
;-----------------  加载GDT  ----------------
    lgdt [gdt_ptr]
;-----------------  cr0第0位置1  ----------------
    mov eax,cr0
    or eax,0x00000001
    mov cr0,eax

   ;jmp dword SELECTOR_CODE:p_mode_start
    jmp SELECTOR_CODE:p_mode_start                      ; 刷新流水线，避免分支预测的影响,这种cpu优化策略，最怕jmp跳转，
					                                         ; 这将导致之前做的预测失效，从而起到了刷新的作用。
.error_hlt:                                            ;出错则挂起
    hlt

[bits 32]
p_mode_start:
    mov ax,SELECTOR_DATA
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov esp,LOADER_STACK_TOP
    mov ax,SELECTOR_VIDEO
    mov gs,ax

; -------------------------   加载kernel  ----------------------
    mov eax, KERNEL_START_SECTOR                        ; kernel.bin所在的扇区号
    mov ebx, KERNEL_BIN_BASE_ADDR                       ; 从磁盘读出后，写入到ebx指定的地址
    mov ecx, 200                                        ; 读入的扇区数
    call rd_disk_m_32

    call setup_page                                     ;创建页目录表的函数,我们的页目录表必须放在1M开始的位置，所以必须在开启保护模式后运行
   ;以下两句是将gdt描述符中视频段描述符中的段基址+0xc0000000
    mov ebx, [gdt_ptr + 2]                              ;ebx中存着GDT_BASE
    or dword [ebx + 0x18 + 4], 0xc0000000               ;视频段是第3个段描述符,每个描述符是8字节,故0x18 = 24，然后+4，是取出了视频段段描述符的高4字节。然后or操作，段基址最高位+c

    add dword [gdt_ptr + 2], 0xc0000000                 ;将gdt的基址加上0xc0000000使其成为内核所在的高地址

    add esp, 0xc0000000                                 ; 将栈指针同样映射到内核地址

    mov eax, PAGE_DIR_TABLE_POS                         ; 把页目录地址赋给cr3
    mov cr3, eax
                                                        
    mov eax, cr0                                        ; 打开cr0的pg位(第31位)
    or eax, 0x80000000
    mov cr0, eax

    lgdt [gdt_ptr]                                      ;在开启分页后,用gdt新的地址重新加载

enter_kernel:
    call kernel_init
    mov esp, 0xc009f000
    jmp KERNEL_ENTRY_POINT                              ; 用地址0x1500访问测试，结果ok

;-----------------   将kernel.bin中的segment拷贝到编译的地址   -----------
kernel_init:
    xor eax, eax
    xor ebx, ebx
    xor ecx, ecx
    xor edx, edx

    mov dx, [KERNEL_BIN_BASE_ADDR + 42]
    mov ebx, [KERNEL_BIN_BASE_ADDR + 28]
    add ebx, KERNEL_BIN_BASE_ADDR
    mov cx, [KERNEL_BIN_BASE_ADDR + 44]
.each_segment:
    cmp byte [ebx + 0], PT_NULL
    je .PTNULL

    push dword [ebx + 16]
    mov eax, [ebx + 4]
    add eax, KERNEL_BIN_BASE_ADDR
    push eax
    push dword [ebx + 8]
    call mem_cpy
    add esp,12
.PTNULL:
   add ebx, edx
   loop .each_segment
   ret

;----------  逐字节拷贝 mem_cpy(dst,src,size) ------------
;输入:栈中三个参数(dst,src,size)
;输出:无
;---------------------------------------------------------
mem_cpy:
    cld
    push ebp
    mov ebp, esp
    push ecx
    mov edi, [ebp + 8]
    mov esi, [ebp + 12]
    mov ecx, [ebp + 16]
    rep movsb
   ;恢复环境
    pop ecx
    pop ebp
    ret


setup_page:
;------------------------------------------   创建页目录及页表  -------------------------------------   
;----------------以下6行是将1M开始的4KB置为0，将页目录表初始化
    mov ecx, 4096                                       ;创建4096个byte 0，循环4096次
    mov esi, 0                                          ;用esi来作为偏移量寻址
.clear_page_dir:
    mov byte [PAGE_DIR_TABLE_POS + esi], 0
    inc esi
    loop .clear_page_dir

; ----------------初始化页目录表，让0号项与768号指向同一个页表，该页表管理从0开始4M的空间
.create_pde:
;一个页目录表项可表示4MB内存,这样0xc03fffff以下的地址和0x003fffff以下的地址都指向相同的页表，这是为将地址映射为内核地址做准备
    mov eax, PAGE_DIR_TABLE_POS
    add eax, 0x1000
    mov ebx, eax
    or eax, PG_US_U | PG_RW_W | PG_P
    mov [PAGE_DIR_TABLE_POS + 0x0], eax
    mov [PAGE_DIR_TABLE_POS + 0xc00], eax
    sub eax, 0x1000
    ;----------------- 使最后一个目录项指向页目录表自己的地址，为的是将来动态操作页表做准备
    mov [PAGE_DIR_TABLE_POS + 4092], eax

    mov ecx, 256
    ; -----------------初始化第一个页表，因为我们的操作系统不会超过1M，所以只用初始化256项
    mov esi, 0
    mov edx, PG_US_U | PG_RW_W | PG_P
.create_pte:
    mov [ebx+esi*4],edx
    add edx,4096
    inc esi
    loop .create_pte
; -------------------初始化页目录表769号-1022号项，769号项指向第二个页表的地址（此页表紧挨着上面的第一个页表），770号指向第三个，以此类推
    mov eax, PAGE_DIR_TABLE_POS 
    add eax, 0x2000
    or eax, PG_US_U | PG_RW_W | PG_P
    mov ebx, PAGE_DIR_TABLE_POS
    mov ecx, 254
    mov esi, 769
.create_kernel_pde:
    mov [ebx+esi*4], eax
    inc esi
    add eax, 0x1000
    loop .create_kernel_pde
    ret
;-------------------------------------------------------------------------------
;功能:读取硬盘n个扇区
rd_disk_m_32:
;-------------------------------------------------------------------------------
    mov esi,eax
    mov di,cx
    mov dx,0x1f2
    mov al,cl
    out dx,al
    mov eax,esi
    mov dx,0x1f3
    out dx,al
    mov cl,8
    shr eax,cl
    mov dx,0x1f4
    out dx,al

    shr eax,cl
    mov dx,0x1f5
    out dx,al

    shr eax,cl
    and al,0x0f
    or al,0xe0
    mov dx,0x1f6
    out dx,al

    mov dx,0x1f7
    mov al,0x20
    out dx,al

.not_ready:
   ;同一端口，写时表示写入命令字，读时表示读入硬盘状态
    nop
    in al,dx
    and al,0x88
    cmp al,0x08
    jnz .not_ready

    mov ax, di
    mov dx, 256
    mul dx
    mov cx, ax
    mov dx, 0x1f0
.go_on_read:
    in ax,dx
    mov [ebx],ax
    add ebx,2
    loop .go_on_read
    ret
