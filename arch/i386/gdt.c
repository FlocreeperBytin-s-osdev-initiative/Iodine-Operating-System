#include "gdt.h"
#include "../../lib/string.h"

extern void gdt_flush(uint32_t gdt_ptr);
extern void tss_flush(void);

#define GDT_NUM_ENTRIES 6

static gdt_entry_t gdt_entries[GDT_NUM_ENTRIES];
static gdt_ptr_t   gdt_ptr;
static tss_entry_t tss_entry;

static void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;

    gdt_entries[num].granularity |= gran & 0xF0;
    gdt_entries[num].access      = access;
}

static void write_tss(int num, uint16_t ss0, uint32_t esp0) {
    uint32_t base = (uint32_t)&tss_entry;
    uint32_t limit = sizeof(tss_entry) - 1;

    gdt_set_gate(num, base, limit, 0xE9, 0x00);

    memset(&tss_entry, 0, sizeof(tss_entry));
    tss_entry.ss0 = ss0;
    tss_entry.esp0 = esp0;

    // Kernel data segment descriptors
    tss_entry.cs = 0x08 | 0x3;
    tss_entry.ss = 0x10 | 0x3;
    tss_entry.ds = 0x10 | 0x3;
    tss_entry.es = 0x10 | 0x3;
    tss_entry.fs = 0x10 | 0x3;
    tss_entry.gs = 0x10 | 0x3;
    tss_entry.iomap_base = sizeof(tss_entry);
}

void tss_set_stack(uint32_t esp0) {
    tss_entry.esp0 = esp0;
}

void gdt_init(void) {
    gdt_ptr.limit = (sizeof(gdt_entry_t) * GDT_NUM_ENTRIES) - 1;
    gdt_ptr.base  = (uint32_t)&gdt_entries;

    // 0x00: Null segment
    gdt_set_gate(0, 0, 0, 0, 0);

    // 0x08: Kernel Code segment (base 0, 4GB, Ring 0, RX)
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    // 0x10: Kernel Data segment (base 0, 4GB, Ring 0, RW)
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    // 0x18: User Mode Code segment (base 0, 4GB, Ring 3, RX)
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);

    // 0x20: User Mode Data segment (base 0, 4GB, Ring 3, RW)
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    // 0x28: TSS
    write_tss(5, 0x10, 0x90000);

    // Load GDT and flush registers
    gdt_flush((uint32_t)&gdt_ptr);
    tss_flush();
}
