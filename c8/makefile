# BUILD_DIR =./build 
# ENTRY_POINT = 0xc0001500

# AS=nasm
# CC=gcc
# LD=ld

# LIB = -I lib/ -I lib/kernel/ -I lib/user/ -I kernel/ -I device/ 
# # 链接
# ASFLAGS = -f elf32 
# CFLAGS = -Wall $(LTB) -c -fno-builtin -fno-stack-protector -W -Wstrict-prototypes -Wmissing-prototypes

# LDFLAGS = -Ttext $(ENTRY_POINT) -e main -m32 elf_i386 -Map $(BUILD_DIR)/kernel.map
# OBJS = $(BUILD_DIR)/main $(BUILD_DIR)/init.o $(BUILD_DIR)/interrupt.o $(BUILD_DIR)/timer.o \
# $(BUILD_DIR)/kernel.o $(BUILD_DIR)/print.o $(BUILD_DIR)/debug.OBJS

# ## 编译代码



# $(BUILD_DIR)/main.o: kernel/main.c lib/kernel/print.h lib/stdint.h kernel/init.h 
# 	$(CC) $(CFLAGS) $< -o $@ 

# $(BUILD_DIR)/init.o: kernel/init.c kernel/init.h lib/kernel/print.h lib/stdint.h kernel/interrupt.h device/timer.h 
# 	$(CC) $(CFLAGS) $< -o $@

# $(BUILD_DIR)/interrupt.o: kernel/interrupt.c kernel/interrupt.h lib/stdint.h kernel/global.h lib/kernel/io.h lib/kernel/print.h 
# 	$(CC) $(CFLAGS) $< -o $@ 

# $(BUILD_DIR)/timer.o: device/timer.c device/timer.h lib/stdint.h lib/kernel/io.h lib/kernel/print.h 
# 	$(CC) $(CFLAGS) $< -o $@ 

# $(BUILD_DIR)/debug.o: kernel/debug.c kernel/debug.h lib/kernel/print.h lib/stdint.h kernel/interrupt.h 
# 	$(CC) $(CFLAGS) $< -o $@

# ##### 汇编代码编译

# $(BUILD_DIR)/kernel.o: kernel/kernel.S 
# 	$(AS) $(ASFLAGS) $< -o $@

# $(BUILD_DIR)/print.o: lib/kernel/print.S 
# 	$(AS) $(ASFLAGS) $< -o $@

# #### 链接

# $(BUILD_DIR)/kernel.bin: $(OBJS) 
# 	$(LD) $(LDFLAGS) $^ -o $@

# .PHONY: mk_dir hd clean all

# mk_dir:
# 	if [[ ! -d $(BUILD_DIR) ]]; then mkdir $(BUILD_DIR); fi

# hd:
# 	dd if =$(BUILD_DIR)/kernel.bin of=/home/yunjian/os/HD60.img bs=512 count=200 seek=9 conv=notrunc

# clean:
# 	cd $(BUILD_DIR) && rm -rf $(BUILD_DIR)

# build: $(BUILD_DIR)/kernel.bin

# all: mk_dir build hd


# 定义变量
AS = nasm
CC = gcc
LD = ld
DD = dd
ASFLAGS = -f elf
CFLAGS = -m32 -I lib/kernel/ -I lib/ -I kernel/ -I device/ -fno-builtin -fno-stack-protector
LDFLAGS = -m elf_i386 -Ttext 0xc0001500 -e main

# 文件路径
BOOT_DIR = boot
BUILD_DIR = build
DEVICE_DIR = device
KERNEL_DIR = kernel
LIB_DIR = lib
IMG = /home/yunjian/os/HD60.img

# 目标文件
KERNEL_OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/init.o $(BUILD_DIR)/interrupt.o \
              $(BUILD_DIR)/print.o $(BUILD_DIR)/kernel.o $(BUILD_DIR)/timer.o  $(BUILD_DIR)/debug.o \
            #   $(BUILD_DIR)/debug.o $(BUILD_DIR)/memory.o $(BUILD_DIR)/bitmap.o \
			#   $(BUILD_DIR)/string.o

# 默认目标
all: $(IMG)

# 编译 MBR 和 Loader
$(BOOT_DIR)/mbr.bin: $(BOOT_DIR)/mbr.S
	$(AS) -I $(BOOT_DIR)/include/ -o $@ $<

$(BOOT_DIR)/loader.bin: $(BOOT_DIR)/loader.S
	$(AS) -I $(BOOT_DIR)/include/ -o $@ $<

# 编译内核源文件
$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: $(DEVICE_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: $(LIB_DIR)/kernel/%.S
	$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: $(LIB_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# $(BUILD_DIR)/bitmap.o: $(LIB_DIR)/kernel/bitmap.c
# 	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/kernel.o: $(KERNEL_DIR)/kernel.S
	$(AS) $(ASFLAGS) -o $@ $<

# 链接内核
$(BUILD_DIR)/kernel.bin: $(KERNEL_OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

# 写入磁盘镜像
$(IMG): $(BOOT_DIR)/mbr.bin $(BOOT_DIR)/loader.bin $(BUILD_DIR)/kernel.bin
	$(DD) if=$(BOOT_DIR)/mbr.bin of=$(IMG) bs=512 count=1 conv=notrunc
	$(DD) if=$(BOOT_DIR)/loader.bin of=$(IMG) bs=512 count=4 seek=2 conv=notrunc
	$(DD) if=$(BUILD_DIR)/kernel.bin of=$(IMG) bs=512 count=200 seek=9 conv=notrunc

# 启动 Bochs 模拟器
bochs:
	bochs -f /home/yunjian/os/bochsrc.disk -q

# 清理生成的文件
clean:
	rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/kernel.bin $(BOOT_DIR)/*.bin