;------------- loader 和 kernel ---------- 
LOADER_BASE_ADDR equ 0x900 
LOADER_START_SECTOR equ 0x2 
;-------------- gdt 描述符属性 -------------

DESC_G_4K equ 1_00000000000000000000000b 
DESC_D_32 equ 1_0000000000000000000000b 
DESC_L equ 0_000000000000000000000b 
; 64 位代码标记,此处标记为 0 便可

DESC_AVL equ 0_00000000000000000000b 
 ; CPU 不用此位,暂置为 0 

DESC_LIMIT_CODE2 equ 1111_0000000000000000b 
DESC_LIMIT_DATA2 equ DESC_LIMIT_CODE2 
DESC_LIMIT_VIDEO2 equ 0000_0000000000000000b 

DESC_P equ 1_000000000000000b 
DESC_DPL_0 equ 00_0000000000000b 
DESC_DPL_1 equ 01_0000000000000b 
DESC_DPL_2 equ 10_0000000000000b 
DESC_DPL_3 equ 11_0000000000000b 
DESC_S_CODE equ 1_000000000000b 
DESC_S_DATA equ DESC_S_CODE 
DESC_S_sys equ 0_000000000000b 
DESC_TYPE_CODE equ 1000_00000000b 
;x=1,c=0,r=0,a=0 代码段是可执行的，非一致性，不可读，已访问位 a 清 0 
DESC_TYPE_DATA equ 0010_00000000b 
;x=0,e=0,w=1,a=0 数据段是不可执行的，向上扩展的，可写，已访问位 a 清 0 
 
DESC_CODE_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_CODE2 + DESC_P+DESC_DPL_0 + DESC_S_CODE +DESC_TYPE_CODE + 0x00 
DESC_DATA_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 +DESC_L + DESC_AVL + DESC_LIMIT_DATA2 + DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x00 
DESC_VIDEO_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_VIDEO2 + DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x0B

; 说这里0x0B而不是0x00是因为在实模式下，视频内存的段地址是0xB8000，而0x0B是0xB8000的段选择子

; DESC_CODE_HIDESC_D_32DESC_D_32H4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_CODE2 + DESC_P + DESC_DPL_0 + DESC_S_CODE + DESC_TYPE_CODE + 0x00
; DESC_DATA_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_DATA2 + DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x00
; DESC_VIDEO_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_VIDEO2 + DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x0B

;-------------- 选择子属性 --------------- 
RPL0 equ 00b 
RPL1 equ 01b 
RPL2 equ 10b 
RPL3 equ 11b 
TI_GDT equ 000b 
TI_LDT equ 100b 




; ;-------------	 loader和kernel   ----------
; LOADER_BASE_ADDR equ 0x900 
; LOADER_START_SECTOR equ 0x2

; ;--------------   模块化的gdt描述符字段宏-------------
; DESC_G_4K   equ	  1_00000000000000000000000b        ;设置段界限的单位为4KB
; DESC_D_32   equ	   1_0000000000000000000000b        ;设置代码段/数据段的有效地址（段内偏移地址）及操作数大小为32位，而非16位
; DESC_L	    equ	    0_000000000000000000000b        ;64位代码段标记位，我们现在是在编写32位操作系统，此处标记为0便可。
; DESC_AVL    equ	     0_00000000000000000000b        ;此标志位是为了给操作系统或其他软件设计的一个自定义位，
;                                                     ;可以将这个位用于任何自定义的需求。
;                                                     ;比如，操作系统可以用这个位来标记这个段是否正在被使用，或者用于其他特定的需求。
;                                                     ;这取决于开发者如何使用这个位。但从硬件的角度来看，AVL位没有任何特定的功能或意义，它的使用完全由软件决定。
; DESC_LIMIT_CODE2  equ 1111_0000000000000000b        ;定义代码段要用的段描述符高32位中16~19段界限为全1
; DESC_LIMIT_DATA2  equ DESC_LIMIT_CODE2              ;定义数据段要用的段描述符高32位中16~19段界限为全1
; DESC_LIMIT_VIDEO2  equ 0000_0000000000000000b        ;定义我们要操作显存时对应的段描述符的高32位中16~19段界限为全0
; DESC_P	    equ		  1_000000000000000b            ;定义了段描述符中的P标志位，表示该段描述符指向的段是否在内存中
; DESC_DPL_0  equ		   00_0000000000000b            ;定义DPL为0的字段
; DESC_DPL_1  equ		   01_0000000000000b            ;定义DPL为1的字段
; DESC_DPL_2  equ		   10_0000000000000b            ;定义DPL为2的字段
; DESC_DPL_3  equ		   11_0000000000000b            ;定义DPL为3的字段
; DESC_S_CODE equ		     1_000000000000b            ;无论代码段，还是数据段，对于cpu来说都是非系统段，所以将S位置为1，见书p153图
; DESC_S_DATA equ	  DESC_S_CODE                       ;无论代码段，还是数据段，对于cpu来说都是非系统段，所以将S位置为1，见书p153图
; DESC_S_sys  equ		     0_000000000000b            ;将段描述符的S位置为0，表示系统段
; DESC_TYPE_CODE  equ	      1000_00000000b            ;x=1,c=0,r=0,a=0 代码段是可执行的,非依从的,不可读的,已访问位a清0.  
; DESC_TYPE_DATA  equ	      0010_00000000b            ;x=0,e=0,w=1,a=0 数据段是不可执行的,向上扩展的,可写的,已访问位a清0.

; ;定义代码段，数据段，显存段的高32位
; DESC_CODE_HIDESC_D_32DESC_D_32H4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_CODE2 + DESC_P + DESC_DPL_0 + DESC_S_CODE + DESC_TYPE_CODE + 0x00
; DESC_DATA_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_DATA2 + DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x00
; DESC_VIDEO_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_VIDEO2 + DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x0B

; ;--------------   模块化的选择子字段宏  ---------------
; RPL0  equ   00b                                     ;定义选择字的RPL为0
; RPL1  equ   01b                                     ;定义选择子的RPL为1
; RPL2  equ   10b                                     ;定义选择字的RPL为2
; RPL3  equ   11b                                     ;定义选择子的RPL为3
; TI_GDT	 equ   000b                                 ;定义段选择子请求的段描述符是在GDT中
; TI_LDT	 equ   100b                                 ;定义段选择子请求的段描述符是在LDT中








