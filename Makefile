# Makefile for Iodine Operating System
# Pure custom x86 bare-metal operating system

CC = gcc
CFLAGS = -m32 -ffreestanding -fno-pie -fno-builtin -fno-stack-protector -Wall -Wextra -Werror -O2 -Iinclude -Ilib
AS = as
ASFLAGS = --32
LD = ld
LDFLAGS = -m elf_i386 -T boot/linker.ld -nostdlib -z noexecstack
OBJCOPY = objcopy

BIN_DIR = bin
OBJ_DIR = obj

# Object files
OBJS = \
	$(OBJ_DIR)/boot/multiboot.o \
	$(OBJ_DIR)/arch/gdt_flush.o \
	$(OBJ_DIR)/arch/interrupts.o \
	$(OBJ_DIR)/kernel/task_switch.o \
	$(OBJ_DIR)/arch/gdt.o \
	$(OBJ_DIR)/arch/idt.o \
	$(OBJ_DIR)/arch/pic.o \
	$(OBJ_DIR)/arch/isr.o \
	$(OBJ_DIR)/drivers/vga.o \
	$(OBJ_DIR)/drivers/serial.o \
	$(OBJ_DIR)/drivers/timer.o \
	$(OBJ_DIR)/drivers/keyboard.o \
	$(OBJ_DIR)/drivers/rtc.o \
	$(OBJ_DIR)/drivers/speaker.o \
	$(OBJ_DIR)/drivers/ata.o \
	$(OBJ_DIR)/drivers/power.o \
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
	$(OBJ_DIR)/apps/calc.o

all: $(BIN_DIR)/iodine.img $(BIN_DIR)/iodine.iso $(BIN_DIR)/iodine.elf

directories:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(OBJ_DIR)/boot $(OBJ_DIR)/arch $(OBJ_DIR)/drivers $(OBJ_DIR)/mm $(OBJ_DIR)/kernel $(OBJ_DIR)/fs $(OBJ_DIR)/lib $(OBJ_DIR)/shell $(OBJ_DIR)/apps

# Assemble 16-bit MBR bootloader
$(BIN_DIR)/boot.bin: boot/boot.s | directories
	@$(AS) $(ASFLAGS) boot/boot.s -o $(OBJ_DIR)/boot/boot.o
	@$(LD) -m elf_i386 -Ttext 0x7c00 --oformat binary $(OBJ_DIR)/boot/boot.o -o $@

# Assemble 32-bit Multiboot & context switch routines
$(OBJ_DIR)/boot/%.o: boot/%.s | directories
	@$(AS) $(ASFLAGS) $< -o $@

$(OBJ_DIR)/arch/%.o: arch/i386/%.s | directories
	@$(AS) $(ASFLAGS) $< -o $@

$(OBJ_DIR)/kernel/%.o: kernel/%.s | directories
	@$(AS) $(ASFLAGS) $< -o $@

# Compile C sources
$(OBJ_DIR)/arch/%.o: arch/i386/%.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/drivers/%.o: drivers/%.c | directories
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

$(OBJ_DIR)/apps/%.o: apps/%.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

# Link kernel ELF
$(BIN_DIR)/iodine.elf: $(OBJS) | directories
	$(LD) $(LDFLAGS) $(OBJS) -o $@

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
