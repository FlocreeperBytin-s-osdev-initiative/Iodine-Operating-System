#include <types.h>
#include "../boot/multiboot.h"
#include "../arch/i386/io.h"
#include "../arch/i386/gdt.h"
#include "../arch/i386/idt.h"
#include "../arch/i386/isr.h"
#include "../arch/i386/pic.h"
#include "../drivers/vga.h"
#include "../drivers/serial.h"
#include "../drivers/timer.h"
#include "../drivers/keyboard.h"
#include "../drivers/rtc.h"
#include "../drivers/speaker.h"
#include "../drivers/ata.h"
#include "../mm/pmm.h"
#include "../mm/vmm.h"
#include "../mm/kheap.h"
#include "../fs/vfs.h"
#include "../fs/ramfs.h"
#include "../fs/devfs.h"
#include "../fs/procfs.h"
#include "process.h"
#include "sched.h"
#include "syscall.h"
#include "../shell/ish.h"
#include "../shell/commands.h"
#include "../lib/stdio.h"

void kernel_main(uint32_t magic, multiboot_info_t *mboot_info) {
    // 1. Initialize text video mode
    vga_init();

    // 2. Initialize serial COM1 output
    serial_init();

    // 3. Display boot splash
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    printf("===============================================================================\n");
    printf("                    IODINE OPERATING SYSTEM - KERNEL v1.0.0                    \n");
    printf("===============================================================================\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    // 4. Setup GDT and TSS
    printf("[BOOT] Initializing GDT and Task State Segment...\n");
    gdt_init();

    // 5. Setup IDT and CPU Exceptions
    printf("[BOOT] Initializing IDT and Exception Handlers...\n");
    idt_init();
    isr_init();

    // 6. Remap 8259A PIC
    printf("[BOOT] Remapping 8259A Programmable Interrupt Controller...\n");
    pic_init();

    // 7. Setup PIT Timer
    printf("[BOOT] Initializing Programmable Interval Timer (100 Hz)...\n");
    timer_init(100);

    // 8. Setup PS/2 Keyboard Driver
    printf("[BOOT] Initializing PS/2 Keyboard Controller...\n");
    keyboard_init();

    // 9. Setup CMOS / RTC
    printf("[BOOT] Initializing Real Time Clock (CMOS)...\n");
    rtc_init();

    // 10. Setup ATA PIO Driver
    printf("[BOOT] Initializing ATA / IDE Controller...\n");
    ata_init();

    // 11. Memory Management
    uint32_t mem_kb = 128 * 1024; // Default 128 MB
    if (magic == MULTIBOOT_BOOTLOADER_MAGIC && mboot_info && (mboot_info->flags & 1)) {
        mem_kb = mboot_info->mem_lower + mboot_info->mem_upper;
    }
    printf("[BOOT] Initializing Physical Memory Manager (%u MB RAM)...\n", mem_kb / 1024);
    pmm_init(mem_kb);

    printf("[BOOT] Initializing Virtual Memory Manager (Paging)...\n");
    vmm_init();

    printf("[BOOT] Initializing Dynamic Kernel Heap...\n");
    kheap_init();

    // 12. Virtual File System
    printf("[BOOT] Initializing Virtual File System (VFS)...\n");
    vfs_init();

    printf("[BOOT] Mounting RamFS as root '/'...\n");
    vfs_node_t *root_fs = ramfs_init();
    vfs_mount("/", root_fs);

    printf("[BOOT] Mounting DevFS at '/dev'...\n");
    vfs_node_t *dev_fs = devfs_init();
    vfs_mount("/dev", dev_fs);

    printf("[BOOT] Mounting ProcFS at '/proc'...\n");
    vfs_node_t *proc_fs = procfs_init();
    vfs_mount("/proc", proc_fs);

    // 13. System Calls
    printf("[BOOT] Registering System Call Gate (int 0x80)...\n");
    syscall_init();

    // 14. Multitasking & Scheduler
    printf("[BOOT] Initializing Process Manager and Scheduler...\n");
    process_init();
    sched_init();

    // 15. Commands
    commands_init();

    printf("[BOOT] Boot sequence complete! Enabling hardware interrupts.\n");
    sti();

    // Spawn interactive shell directly or as task
    printf("[BOOT] Launching Iodine Shell (ish)...\n\n");
    shell_main();

    // Fallback idle loop
    while (1) {
        hlt();
    }
}
