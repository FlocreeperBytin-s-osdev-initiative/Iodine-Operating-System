# Makefile for Iodine Operating System
# Pure custom x86 / x86_64 bare-metal operating system

CC = gcc
CFLAGS = -m32 -ffreestanding -fno-pie -fno-builtin -fno-stack-protector -Wall -Wextra -Werror -O2 -Iinclude -Ilib -I.
CFLAGS64 = -m64 -ffreestanding -fno-pie -fno-builtin -fno-stack-protector -Wall -Wextra -Werror -O2 -Iinclude -Ilib -I.
AS = as
ASFLAGS = --32
ASFLAGS64 = --64
LD = ld
LDFLAGS = -m elf_i386 -T boot/linker.ld -nostdlib -z noexecstack
LDFLAGS64 = -m elf_x86_64 -nostdlib -z noexecstack
OBJCOPY = objcopy

BIN_DIR = bin
OBJ_DIR = obj

OBJS = \
	$(OBJ_DIR)/boot/multiboot.o \
	$(OBJ_DIR)/arch/gdt_flush.o \
	$(OBJ_DIR)/arch/interrupts.o \
	$(OBJ_DIR)/kernel/task_switch.o \
	$(OBJ_DIR)/arch/gdt.o \
	$(OBJ_DIR)/arch/idt.o \
	$(OBJ_DIR)/arch/pic.o \
	$(OBJ_DIR)/arch/isr.o \
	$(OBJ_DIR)/arch/kernel64.o \
	$(OBJ_DIR)/drivers/vga.o \
	$(OBJ_DIR)/drivers/serial.o \
	$(OBJ_DIR)/drivers/timer.o \
	$(OBJ_DIR)/drivers/keyboard.o \
	$(OBJ_DIR)/drivers/rtc.o \
	$(OBJ_DIR)/drivers/speaker.o \
	$(OBJ_DIR)/drivers/ata.o \
	$(OBJ_DIR)/drivers/power.o \
	$(OBJ_DIR)/drivers/virtio_pci.o \
	$(OBJ_DIR)/drivers/virtqueue.o \
	$(OBJ_DIR)/drivers/virtio_blk.o \
	$(OBJ_DIR)/drivers/virtio_net.o \
	$(OBJ_DIR)/compat/posix.o \
	$(OBJ_DIR)/compat/linux_sys.o \
	$(OBJ_DIR)/compat/musl_compat.o \
	$(OBJ_DIR)/compat/ldk.o \
	$(OBJ_DIR)/compat/ldk_bridge.o \
	$(OBJ_DIR)/compat/linux_env.o \
	$(OBJ_DIR)/compat/linux_e1000.o \
	$(OBJ_DIR)/mm/pmm.o \
	$(OBJ_DIR)/mm/vmm.o \
	$(OBJ_DIR)/mm/kheap.o \
	$(OBJ_DIR)/fs/vfs.o \
	$(OBJ_DIR)/fs/ramfs.o \
	$(OBJ_DIR)/fs/devfs.o \
	$(OBJ_DIR)/fs/procfs.o \
	$(OBJ_DIR)/kernel/main.o \
	$(OBJ_DIR)/kernel/process.o \
	$(OBJ_DIR)/kernel/sched.o \
	$(OBJ_DIR)/kernel/syscall.o \
	$(OBJ_DIR)/kernel/panic.o \
	$(OBJ_DIR)/lib/string.o \
	$(OBJ_DIR)/lib/ctype.o \
	$(OBJ_DIR)/lib/stdlib.o \
	$(OBJ_DIR)/lib/stdio.o \
	$(OBJ_DIR)/shell/commands.o \
	$(OBJ_DIR)/shell/ish.o \
	$(OBJ_DIR)/apps/snake.o \
	$(OBJ_DIR)/apps/editor.o \
	$(OBJ_DIR)/apps/matrix.o \
	$(OBJ_DIR)/apps/calc.o \
	$(OBJ_DIR)/apps/apk.o \
	$(OBJ_DIR)/apps/cal.o \
	$(OBJ_DIR)/apps/hexdump.o \
	$(OBJ_DIR)/apps/column.o \
	$(OBJ_DIR)/apps/banner.o \
	$(OBJ_DIR)/apps/morse.o \
	$(OBJ_DIR)/apps/cksum.o

all: $(BIN_DIR)/iodine.img $(BIN_DIR)/iodine.iso $(BIN_DIR)/iodine.elf $(BIN_DIR)/iodine64.elf

directories:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(OBJ_DIR)/boot $(OBJ_DIR)/arch $(OBJ_DIR)/drivers $(OBJ_DIR)/compat $(OBJ_DIR)/mm $(OBJ_DIR)/kernel $(OBJ_DIR)/fs $(OBJ_DIR)/lib $(OBJ_DIR)/shell $(OBJ_DIR)/apps

# 16-bit MBR Bootloader
$(BIN_DIR)/boot.bin: boot/boot.s | directories
	@$(AS) $(ASFLAGS) boot/boot.s -o $(OBJ_DIR)/boot/boot.o
	@$(LD) -m elf_i386 -Ttext 0x7c00 --oformat binary $(OBJ_DIR)/boot/boot.o -o $@

# 32-bit Multiboot & context switch routines
$(OBJ_DIR)/boot/%.o: boot/%.s | directories
	@$(AS) $(ASFLAGS) $< -o $@

$(OBJ_DIR)/arch/%.o: arch/i386/%.s | directories
	@$(AS) $(ASFLAGS) $< -o $@

$(OBJ_DIR)/kernel/%.o: kernel/%.s | directories
	@$(AS) $(ASFLAGS) $< -o $@

# Compile C sources
$(OBJ_DIR)/arch/kernel64.o: arch/x86_64/kernel64.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/arch/%.o: arch/i386/%.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/drivers/virtio_pci.o: drivers/virtio/virtio_pci.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/drivers/virtqueue.o: drivers/virtio/virtqueue.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/drivers/virtio_blk.o: drivers/virtio/virtio_blk.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/drivers/virtio_net.o: drivers/virtio/virtio_net.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/drivers/%.o: drivers/%.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/compat/posix.o: compat/posix/posix.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/compat/linux_sys.o: compat/linux/linux_sys.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/compat/musl_compat.o: compat/musl/musl_compat.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/compat/ldk.o: compat/linux_driver_sandbox/ldk.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/compat/ldk_bridge.o: compat/linux_driver_sandbox/ldk_bridge.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/compat/linux_env.o: compat/linux_driver_sandbox/linux_env.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/compat/linux_e1000.o: compat/linux_driver_sandbox/drivers/linux_e1000.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/mm/%.o: mm/%.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/fs/%.o: fs/%.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/kernel/%.o: kernel/%.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/lib/%.o: lib/%.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/shell/%.o: shell/%.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/apps/apk.o: apps/apk/apk.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/apps/cal.o: apps/bsdutils/cal.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/apps/hexdump.o: apps/bsdutils/hexdump.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/apps/column.o: apps/bsdutils/column.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/apps/banner.o: apps/bsdutils/banner.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/apps/morse.o: apps/bsdutils/morse.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/apps/cksum.o: apps/bsdutils/cksum.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/apps/%.o: apps/%.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

# Link 32-bit Multiboot Kernel
$(BIN_DIR)/iodine.elf: $(OBJS) | directories
	$(LD) $(LDFLAGS) $(OBJS) -o $@

# Link 64-bit Kernel ELF target
$(BIN_DIR)/iodine64.elf: | directories
	@gcc $(CFLAGS64) -c arch/x86_64/kernel64.c -o $(OBJ_DIR)/arch/kernel64_native.o
	@ld $(LDFLAGS64) -r $(OBJ_DIR)/arch/kernel64_native.o -o $@
	@echo "Built 64-bit ELF kernel: $@"

# Convert ELF to flat binary for MBR loader
$(BIN_DIR)/iodine-kernel.bin: $(BIN_DIR)/iodine.elf
	$(OBJCOPY) -O binary $< $@

# Create 1.44MB Floppy Image
$(BIN_DIR)/iodine.img: $(BIN_DIR)/boot.bin $(BIN_DIR)/iodine-kernel.bin
	@dd if=/dev/zero of=$@ bs=1024 count=1440 2>/dev/null
	@dd if=$(BIN_DIR)/boot.bin of=$@ conv=notrunc 2>/dev/null
	@dd if=$(BIN_DIR)/iodine-kernel.bin of=$@ seek=1 conv=notrunc 2>/dev/null
	@echo "Built bootable disk image: $@"

# Create El Torito Bootable ISO
$(BIN_DIR)/iodine.iso: $(BIN_DIR)/iodine.img tools/mkiso.py
	@python3 tools/mkiso.py $(BIN_DIR)/iodine.img $@

clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR)

run: all
	@node web/run.js

test: all
	@./test.sh

.PHONY: all directories clean run test
