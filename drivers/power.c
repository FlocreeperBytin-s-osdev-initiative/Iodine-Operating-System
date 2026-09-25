#include "power.h"
#include "../arch/i386/io.h"
#include "vga.h"
#include "../lib/stdio.h"

void power_reboot(void) {
    printf("\nRebooting system...\n");
    cli();

    // Pulse the CPU reset line via 8042 keyboard controller
    uint8_t good = 0x02;
    while (good & 0x02) {
        good = inb(0x64);
    }
    outb(0x64, 0xFE);

    // If that fails, triple fault via invalid IDT
    struct {
        uint16_t limit;
        uint32_t base;
    } __attribute__((packed)) null_idt = { 0, 0 };
    __asm__ volatile ("lidt %0; int3" : : "m"(null_idt));

    while (1) hlt();
}

void power_shutdown(void) {
    printf("\nShutting down system...\n");
    cli();

    // QEMU / Bochs / VirtualBox ACPI shutdown
    outw(0x604, 0x2000);
    outw(0xB004, 0x2000);
    outw(0x4004, 0x3400);

    // APM shutdown via port 0x8900
    outw(0x8900, 0x2000);

    // If still alive, halt
    printf("It is now safe to turn off your computer.\n");
    while (1) {
        hlt();
    }
}
