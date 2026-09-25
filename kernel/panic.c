#include "panic.h"
#include "../drivers/vga.h"
#include "../drivers/serial.h"
#include "../lib/stdio.h"
#include "../arch/i386/io.h"

void kernel_panic(const char *message) {
    cli();

    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
    vga_clear();

    printf("\n");
    printf("  ===============================================================\n");
    printf("                     *** KERNEL PANIC ***                        \n");
    printf("                         IODINE OS                               \n");
    printf("  ===============================================================\n\n");
    printf("  Fatal Error: %s\n\n", message);
    printf("  The system has halted to prevent data corruption.\n");
    printf("  Please reboot the virtual machine or computer.\n\n");
    printf("  ===============================================================\n");

    while (1) {
        hlt();
    }
}

void kernel_panic_registers(const char *message, registers_t *regs) {
    cli();

    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
    vga_clear();

    uint32_t cr2 = 0;
    __asm__ volatile ("mov %%cr2, %0" : "=r"(cr2));

    printf("\n");
    printf("  ===============================================================\n");
    printf("                     *** KERNEL EXCEPTION ***                    \n");
    printf("                         IODINE OS                               \n");
    printf("  ===============================================================\n\n");
    printf("  Fault: %s (Interrupt #%u, Error Code: 0x%x)\n\n", message, regs->int_no, regs->err_code);
    printf("  Registers:\n");
    printf("    EAX: 0x%08x   EBX: 0x%08x   ECX: 0x%08x   EDX: 0x%08x\n", regs->eax, regs->ebx, regs->ecx, regs->edx);
    printf("    ESI: 0x%08x   EDI: 0x%08x   EBP: 0x%08x   ESP: 0x%08x\n", regs->esi, regs->edi, regs->ebp, regs->esp);
    printf("    EIP: 0x%08x   CS:  0x%04x       DS:  0x%04x       SS:  0x%04x\n", regs->eip, regs->cs, regs->ds, regs->ss);
    printf("    EFLAGS: 0x%08x    CR2: 0x%08x\n\n", regs->eflags, cr2);
    printf("  System execution halted.\n");
    printf("  ===============================================================\n");

    while (1) {
        hlt();
    }
}
