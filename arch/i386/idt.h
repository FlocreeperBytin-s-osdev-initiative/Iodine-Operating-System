#ifndef ARCH_I386_IDT_H
#define ARCH_I386_IDT_H

#include <types.h>

/* IDT entry structure */
struct idt_entry_struct {
    uint16_t base_low;
    uint16_t sel;
    uint8_t  always0;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed));
typedef struct idt_entry_struct idt_entry_t;

/* IDT pointer structure for lidt */
struct idt_ptr_struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));
typedef struct idt_ptr_struct idt_ptr_t;

void idt_init(void);
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

#endif /* ARCH_I386_IDT_H */
