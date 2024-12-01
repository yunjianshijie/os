%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR                  ;加载器的起始位置
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

    ;GDT_SIZE计算了 GDT 的大小，而GDT_LIMIT则是 GDT 的界限值，用于加载到GDTR
    GDT_SIZE equ $ - GDT_BASE
    GDT_LIMIT equ GDT_SIZE - 1
    times 60 dq 0                                       ;此处预留60个描述符的空间，以便将来可以向 GDT 添加其他描述符
    SELECTOR_CODE equ (0x0001<<3) + TI_GDT + RPL0       ;相当于(CODE_DESC - GDT_BASE)/8 + TI_GDT + RPL0
    SELECTOR_DATA equ (0x0002<<3) + TI_GDT + RPL0       ;同上
    SELECTOR_VIDEO equ (0x0003<<3) + TI_GDT + RPL0      ;同上

total_mem_bytes dd 0                                    ;total_mem_bytes用于保存内存容量,以字节为单位
                                                        ;当前偏移loader.bin文件头0x200字节,loader.bin的加载地址是0x900,
                                                        ;故total_mem_bytes内存中的地址是0xb00,以便在内核中引用此地址
gdt_ptr dw GDT_LIMIT                                    ;定义加载进入GDTR的数据，前2字节是gdt界限，后4字节是gdt起始地址
        dd  GDT_BASE

ards_buf times 244 db 0                                 ;total_mem_bytes4字节+gdt_ptr6字节+ards_buf244字节+ards_nr2,共256字节
ards_nr dw 0                                            ;用于记录ards结构体数量

loader_start:
;-------   int 15h eax = 0000E820h ,edx = 534D4150h ('SMAP') 获取内存布局   -------
    xor ebx, ebx                                        ;int 0x15 的0xE820调用在初次调用时要求ebx=0，随后每次调用将会更新ebx的值，直到返回为零
    mov edx, 0x534d4150                                 ;edx只赋值一次，循环体中不会改变,用于确认调用的是 0xE820 功能
    mov di, ards_buf                                    ;di 指向ards_buf缓冲区，用于保存每个内存区域信息结构
.e820_mem_get_loop:                                     ;循环调用中断
    mov eax, 0x0000e820                                 ;执行int 0x15后,eax值变为0x534d4150,所以每次执行int前都要更新为子功能号
    mov ecx, 20                                         ;设置 ARDS 结构大小为20字节
    int 0x15
    add di, cx                                          ;使di增加20字节指向缓冲区中新的ARDS结构位置
    inc word [ards_nr]                                  ;记录ARDS数量
    cmp ebx, 0                                          ;如果ebx != 0，继续循环，否则说明已获取所有内存段信息
    jnz .e820_mem_get_loop
                                                        ;在所有ards结构中，找出(base_add_low + length_low)的最大值，即内存的容量
    mov cx, [ards_nr]                                   ;遍历每一个ARDS结构体,循环次数是ARDS的数量
    mov ebx, ards_buf
    xor edx, edx                                        ;edx清零，将用于存储最大内存容量
.find_max_mem_area:                                     ;遍历并计算最大内存区域.无须判断type是否为1,最大的内存块一定是可被使用
    mov eax, [ebx]                                      ;base_add_low
    add eax, [ebx+8]                                    ;length_low
    add ebx, 20                                         ;指向缓冲区中下一个ARDS结构
    cmp edx, eax                                        ;比较当前内存容量最大值edx和当前区域的容量eax
    jge .next_ards
    mov edx, eax                                        ;如果eax更大，更新edx
.next_ards:
    loop .find_max_mem_area
    mov [total_mem_bytes], edx                          ;将最大内存大小（字节单位）存入 total_mem_bytes
;--------------------   准备进入保护模式   ---------------------
;1 打开A20地址线
;2 加载gdt
;3 将cr0的pe位置1
;--------------------   打开A20   ----------------------------
    in al, 0x92                                         ;读取端口0x92的值到AL
    or al, 0000_0010B                                   ;将A20位置1
    out 0x92,al                                         ;写回端口 0x92，启用A20地址线
;-------------------   加载GDT   -----------------------------
    lgdt [gdt_ptr]                                      ;加载GDT的基址和大小,lgdt指令用于加载GDT的基址和大小，以便CPU可以正确访问内存段
;-------------------   cr0第0位置1   -------------------------
;CR0寄存器是控制寄存器，用于设置CPU的工作模式
    mov eax,cr0                                         ;读取CR0寄存器的当前值
    or eax,0x00000001                                   ;设置PE位
    mov cr0,eax                                         ;更新CR0寄存器
    jmp SELECTOR_CODE:p_mode_start                      ;刷新流水线并跳转到保护模式代码
.error_hlt:                                             ;出错则挂起
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

;------------------------------   加载kernel   ---------------------------------
    mov eax, KERNEL_START_SECTOR                        ;kernel.bin所在的扇区号
    mov ebx, KERNEL_BIN_BASE_ADDR                       ;从磁盘读出后，写入到ebx指定的地址
    mov ecx, 200                                        ;读入的扇区数

    call rd_disk_m_32
    call setup_page                                     ;创建页目录表的函数,我们的页目录表必须放在1M开始的位置，所以必须在开启保护模式后运行

    ;以下两句是将gdt描述符中视频段描述符中的段基址+0xc0000000
    mov ebx, [gdt_ptr + 2]                              ;ebx中存着GDT_BASE
    or dword [ebx + 0x18 + 4], 0xc0000000               ;视频段是第3个段描述符,每个描述符是8字节,故0x18 = 24，然后+4，取出视频段段描述符的高4字节

    add dword [gdt_ptr + 2], 0xc0000000                 ;将gdt的基址加上0xc0000000使其成为内核所在的高地址，使得GDT中的所有段都在高地址范围内
    add esp, 0xc0000000                                 ;将栈指针同样映射到内核地址
    mov eax, PAGE_DIR_TABLE_POS                         ;把页目录地址赋给cr3
    mov cr3, eax
    mov eax, cr0                                        ;打开cr0的pg位(第31位)
    or eax, 0x80000000                                  ;将控制寄存器CR0的第31位设置为1，开启分页
    mov cr0, eax
    lgdt [gdt_ptr]                                      ;在开启分页后,用gdt新的地址重新加载

enter_kernel:
    call kernel_init
    mov esp, 0xc009f000                                 ;设置栈指针esp到一个指定的内存地址0xc009f000，这是内核的栈起始位置
    jmp KERNEL_ENTRY_POINT                              ;跳转到内核入口点 KERNEL_ENTRY_POINT，开始执行内核的主逻辑
kernel_init:
    xor eax, eax                                        ;清空eax
    xor ebx, ebx                                        ;清空ebx, ebx记录程序头表地址
    xor ecx, ecx                                        ;清空ecx, cx记录程序头表中的program header数量
    xor edx, edx                                        ;清空edx, dx 记录program header尺寸
    ;偏移文件42字节处的属性是e_phentsize,表示program header table中每个program header大小
    mov dx, [KERNEL_BIN_BASE_ADDR + 42]
    ;偏移文件开始部分28字节的地方是e_phoff,表示program header table的偏移，ebx中是第1 个program header在文件中的偏移量
    mov ebx, [KERNEL_BIN_BASE_ADDR + 28]
    add ebx, KERNEL_BIN_BASE_ADDR                       ;现在ebx中存着第一个program header的内存地址
    mov cx, [KERNEL_BIN_BASE_ADDR + 44]                 ;偏移文件开始部分44字节的地方是e_phnum,表示有几个program header
.each_segment:
    cmp byte [ebx + 0], PT_NULL                         ;若p_type等于 PT_NULL,说明此program header未使用,跳转到.PTNULL标签
    je .PTNULL

    ;为函数memcpy压入参数,参数是从右往左依然压入.函数原型类似于 memcpy(dst,src,size)
    push dword [ebx + 16]
    mov eax, [ebx + 4]
    add eax, KERNEL_BIN_BASE_ADDR
    push eax
    push dword [ebx + 8]
    call mem_cpy
    add esp,12                                          ;清理栈中压入的三个参数
.PTNULL:
   add ebx, edx                                         ;将ebx移动到下一个program header。edx存储了每个program header的大小。
   loop .each_segment
   ret


mem_cpy:
    cld                                                 ;将FLAG的方向标志位DF清零，rep在执行循环时候si，di就会加1
    push ebp                                            ;将当前的基指针ebp压入栈中，以保存上一个栈帧的信息
    mov ebp, esp                                        ;将当前栈指针esp的值赋给基指针ebp，设置新的栈帧。此时，ebp用于访问函数参数和局部变量
    push ecx                                            ;入栈备份

    mov edi, [ebp + 8]
    mov esi, [ebp + 12]
    mov ecx, [ebp + 16]
    rep movsb                                           ;逐字节拷贝
    ;恢复环境
    pop ecx                                             ;从栈中恢复ecx寄存器的值，恢复之前的状态
    pop ebp                                             ;从栈中恢复ebp的值，结束当前栈帧
    ret


setup_page:
;------------------   将1M开始的4KB置为0
    mov ecx, 4096                                       ;创建4096个byte 0
    mov esi, 0                                          ;用esi来作为偏移量寻址
.clear_page_dir:
    mov byte [PAGE_DIR_TABLE_POS + esi], 0
    inc esi
    loop .clear_page_dir

.create_pde:
;一个页目录表项可表示4MB内存,这样0xc03fffff以下的地址和0x003fffff以下的地址都指向相同的页表，这是为将地址映射为内核地址做准备
    mov eax, PAGE_DIR_TABLE_POS                         ;eax中存着页目录表的位置
    add eax, 0x1000                                     ;将eax指向页目录表后面的第一个页表的起始位置
    mov ebx, eax                                        ;将eax的值复制到ebx，现在ebx中存着第一个页表的起始位置
    or eax, PG_US_U | PG_RW_W | PG_P                    ;页目录项的属性
    mov [PAGE_DIR_TABLE_POS + 0x0], eax                 ;将页目录表的0号项设置为第一个页表的地址和属性
    mov [PAGE_DIR_TABLE_POS + 0xc00], eax               ;将页目录表的768号项也指向第一个页表，实现虚拟地址到物理地址的映射
    sub eax, 0x1000                                     ;将eax减去4K，使其指向页目录表自身的地址
    mov [PAGE_DIR_TABLE_POS + 4092], eax                ;使最后一个目录项指向页目录表自己的地址，为的是将来动态操作页表

    mov ecx, 256                                        ;初始化第一个页表，操作系统不会超过1M，所以初始化256项
    mov esi, 0                                          ;清零，esi来做寻址页表项的偏移量
    mov edx, PG_US_U | PG_RW_W | PG_P	                ;属性为7,US=1,RW=1,P=1
.create_pte:
    mov [ebx+esi*4],edx                                 ;将edx的值（页表项属性）写入第一个页表的esi号位置
    add edx,4096                                        ;将edx指向下一个4KB空间，准备设置下一个页表项
    inc esi                                             ;寻址页表项的偏移量+1
    loop .create_pte                                    ;循环页表的256项,初始化

;----------------   初始化页目录表769号-1022号项，769号项指向第二个页表的地址（此页表紧挨着上面的第一个页表），770号指向第三个，以此类推
    mov eax, PAGE_DIR_TABLE_POS                         ;eax存页目录表的起始位置
    add eax, 0x2000                                     ;此时eax为第二个页表的位置
    or eax, PG_US_U | PG_RW_W | PG_P                    ;设置页目录表项相关属性，现在eax中的值是一个完整的指向第二个页表的页目录表项
    mov ebx, PAGE_DIR_TABLE_POS                         ;ebx现在存着页目录表的起始位置
    mov ecx, 254                                        ;要设置254个表项
    mov esi, 769                                        ;要设置的页目录表项的偏移起始
.create_kernel_pde:
    mov [ebx+esi*4], eax                                ;设置页目录表项
    inc esi
    add eax, 0x1000                                     ;eax指向下一个页表的位置，由于之前设定了属性，所以eax是一个完整的指向下一个页表的页目录表项
    loop .create_kernel_pde
    ret

rd_disk_m_32:
;eax=LBA扇区号
;ebx=将数据写入的内存地址
;ecx=读入的扇区数
    mov esi,eax                                         ;备份eax
    mov di,cx                                           ;备份cx

    ;第1步：选择特定通道的寄存器，设置要读取的扇区数
    mov dx,0x1f2
    mov al,cl                                           ;将要读取的扇区数放入AL寄存器
    out dx,al                                           ;输出到端口
    mov eax,esi                                         ;恢复ax

    ;第2步：在特定通道寄存器中放入要读取扇区的地址，将LBA地址存入0x1f3 ~ 0x1f6
                                                        ;LBA地址7~0位写入端口0x1f3
    mov dx,0x1f3
    out dx,al
                                                        ;LBA地址15~8位写入端口0x1f4
    mov cl,8
    shr eax,cl
    mov dx,0x1f4
    out dx,al
                                                        ;LBA地址23~16位写入端口0x1f5
    shr eax,cl
    mov dx,0x1f5
    out dx,al
    shr eax,cl
    and al,0x0f                                         ;lba第24~27位
    or al,0xe0                                          ;设置7～4位为1110,表示lba模式
    mov dx,0x1f6
    out dx,al

    ;第3步：向0x1f7端口写入读命令，0x20
    mov dx,0x1f7
    mov al,0x20
    out dx,al

    ;第4步：检测硬盘状态
.not_ready:
    ;同一端口，写时表示写入命令字，读时表示读入硬盘状态
    nop
    in al,dx
    and al,0x88                                         ;第4位为1表示硬盘控制器已准备好数据传输，第7位为1表示硬盘忙
    cmp al,0x08
    jnz .not_ready                                      ;若未准备好，继续等

    ;第5步：从0x1f0端口读数据
    mov ax, di                                          ;di当中存储的是要读取的扇区数
    mov dx, 256                                         ;每个扇区512字节，一次读取两个字节，所以一个扇区就要读取256次，与扇区数相乘，就等得到总读取次数
    mul dx                                              ;8位乘法与16位乘法知识查看书p133,注意：16位乘法会改变dx的值！！！！
    mov cx, ax                                          ; 得到了要读取的总次数，然后将这个数字放入cx中
    mov dx, 0x1f0
.go_on_read:
    in ax,dx
    mov [ebx],ax                                        ;与rd_disk_m_16相比，就是把这两句的bx改成了ebx
    add ebx,2
    loop .go_on_read
    ret
