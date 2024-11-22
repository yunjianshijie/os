;-------------	 loader和kernel   ----------
LOADER_BASE_ADDR equ 0x900
LOADER_START_SECTOR equ 0x2

LOADER_STACK_TOP equ LOADER_BASE_ADDR               ;这一条之前是在loader.S中定义，现在搬过来了

KERNEL_BIN_BASE_ADDR equ 0x70000
KERNEL_START_SECTOR equ 0x9
KERNEL_ENTRY_POINT equ 0xc0001500

PAGE_DIR_TABLE_POS equ 0x100000

;--------------   模块化的gdt描述符字段宏-------------
DESC_G_4K   equ	  1_00000000000000000000000b
DESC_D_32   equ	   1_0000000000000000000000b
DESC_L	    equ	    0_000000000000000000000b
DESC_AVL    equ	     0_00000000000000000000b
DESC_LIMIT_CODE2  equ 1111_0000000000000000b
DESC_LIMIT_DATA2  equ DESC_LIMIT_CODE2
DESC_LIMIT_VIDEO2  equ 0000_000000000000000b
DESC_P	    equ		  1_000000000000000b
DESC_DPL_0  equ		   00_0000000000000b
DESC_DPL_1  equ		   01_0000000000000b
DESC_DPL_2  equ		   10_0000000000000b
DESC_DPL_3  equ		   11_0000000000000b
DESC_S_CODE equ		     1_000000000000b
DESC_S_DATA equ	  DESC_S_CODE
DESC_S_sys  equ		     0_000000000000b
DESC_TYPE_CODE  equ	      1000_00000000b
DESC_TYPE_DATA  equ	      0010_00000000b
;定义代码段，数据段，显存段的高32位
DESC_CODE_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_CODE2 + DESC_P + DESC_DPL_0 + DESC_S_CODE + DESC_TYPE_CODE + 0x00
DESC_DATA_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_DATA2 + DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x00
DESC_VIDEO_HIGH4 equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + DESC_L + DESC_AVL + DESC_LIMIT_VIDEO2 + DESC_P + DESC_DPL_0 + DESC_S_DATA + DESC_TYPE_DATA + 0x0b
;--------------   模块化的选择子字段宏  ---------------
RPL0  equ   00b                                     ;定义选择字的RPL为0
RPL1  equ   01b                                     ;定义选择子的RPL为1
RPL2  equ   10b                                     ;定义选择字的RPL为2
RPL3  equ   11b                                     ;定义选择子的RPL为3
TI_GDT	 equ   000b                                 ;定义段选择子请求的段描述符是在GDT中
TI_LDT	 equ   100b                                 ;定义段选择子请求的段描述符是在LDT中
;---------模块化的页目录表字段,PWT PCD A D G AVL 暂时不用设置   ----------
PG_P  equ   1b
PG_RW_R	 equ  00b
PG_RW_W	 equ  10b
PG_US_S	 equ  000b
PG_US_U	 equ  100b
;-------------  程序段的 type 定义   --------------
PT_NULL equ 0