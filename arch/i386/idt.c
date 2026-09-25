#include "idt.h"
#include "../../lib/string.h"

extern void idt_flush(uint32_t idt_ptr);

#define IDT_NUM_ENTRIES 256

static idt_entry_t idt_entries[IDT_NUM_ENTRIES];
static idt_ptr_t   idt_ptr;

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_low  = base & 0xFFFF;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;
    idt_entries[num].sel       = sel;
    idt_entries[num].always0   = 0;
    idt_entries[num].flags     = flags;
}

void idt_init(void) {
    idt_ptr.limit = sizeof(idt_entry_t) * IDT_NUM_ENTRIES - 1;
    idt_ptr.base  = (uint32_t)&idt_entries;

    memset(&idt_entries, 0, sizeof(idt_entry_t) * IDT_NUM_ENTRIES);

    // Call assembly to load IDTR
    idt_flush((uint32_t)&idt_ptr);
}
