#ifndef KERNEL_PANIC_H
#define KERNEL_PANIC_H

#include <types.h>
#include "../arch/i386/isr.h"

void kernel_panic(const char *message);
void kernel_panic_registers(const char *message, registers_t *regs);

#endif /* KERNEL_PANIC_H */
