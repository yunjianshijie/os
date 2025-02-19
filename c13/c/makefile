# 定义变量
AS = nasm
CC = gcc
LD = ld
DD = dd
ASFLAGS = -f elf
CFLAGS = -m32  -I lib/kernel/ -I lib/user/ -I lib/ -I kernel/ -I device/ -I thread/ -I userprog/ -fno-builtin -fno-stack-protector 
LDFLAGS = -m elf_i386 -Ttext 0xc0001500 -e main

# 文件路径
BOOT_DIR = boot
BUILD_DIR = build
DEVICE_DIR = device
KERNEL_DIR = kernel
THREAD_DIR = thread
LIB_DIR = lib
USERPROG_DIR = userprog
IMG = /home/yunjian/os/HD60.img

# 目标文件
KERNEL_OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/init.o $(BUILD_DIR)/interrupt.o \
              $(BUILD_DIR)/print.o $(BUILD_DIR)/kernel.o $(BUILD_DIR)/timer.o   \
              $(BUILD_DIR)/debug.o $(BUILD_DIR)/memory.o $(BUILD_DIR)/bitmap.o \
			  $(BUILD_DIR)/string.o $(BUILD_DIR)/thread.o $(BUILD_DIR)/list.o \
			  $(BUILD_DIR)/switch.o $(BUILD_DIR)/sync.o  $(BUILD_DIR)/console.o \
			  $(BUILD_DIR)/keyboard.o $(BUILD_DIR)/ioqueue.o $(BUILD_DIR)/tss.o \
			  $(BUILD_DIR)/process.o $(BUILD_DIR)/syscall-init.o  $(BUILD_DIR)/syscall.o \
			$(BUILD_DIR)/stdio.o $(BUILD_DIR)/ide.o $(BUILD_DIR)/stdio-kernel.o

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

$(BUILD_DIR)/%.o: $(LIB_DIR)/kernel/%.asm
	$(AS) $(ASFLAGS) -o $@ $<

# $(BUILD_DIR)/%.o: $(LIB_DIR)/kernel/%.S
# 	$(AS) $(ASFLAGS) -o $@ $<


$(BUILD_DIR)/%.o: $(LIB_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/bitmap.o: $(LIB_DIR)/kernel/bitmap.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/syscall.o: $(LIB_DIR)/user/syscall.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/list.o: $(LIB_DIR)/kernel/list.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/stdio-kernel.o: $(LIB_DIR)/kernel/stdio-kernel.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/thread.o: $(THREAD_DIR)/thread.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/sync.o: $(THREAD_DIR)/sync.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/console.o: $(DEVICE_DIR)/console.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/keyboard.o: $(DEVICE_DIR)/keyboard.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/ioqueue.o: $(DEVICE_DIR)/ioqueue.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/tss.o: $(USERPROG_DIR)/tss.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/process.o: $(USERPROG_DIR)/process.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/syscall-init.o: $(USERPROG_DIR)/syscall-init.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/switch.o: $(THREAD_DIR)/switch.asm
	$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/kernel.o: $(KERNEL_DIR)/kernel.asm
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