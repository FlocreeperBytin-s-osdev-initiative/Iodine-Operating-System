#include "vmm.h"
#include "pmm.h"
#include "../lib/string.h"
#include "../lib/stdio.h"
#include "../kernel/panic.h"

// Kernel page directory and 8 identity page tables (32MB)
static page_directory_t kernel_page_directory __attribute__((aligned(4096)));
static page_table_t identity_page_tables[8] __attribute__((aligned(4096)));

static void page_fault_handler(registers_t *regs) {
    uint32_t faulting_address;
    __asm__ volatile("mov %%cr2, %0" : "=r" (faulting_address));

    int present   = regs->err_code & 0x1;
    int rw        = regs->err_code & 0x2;
    int user      = regs->err_code & 0x4;
    int reserved  = regs->err_code & 0x8;
    int id        = regs->err_code & 0x10;

    char msg[128];
    snprintf(msg, sizeof(msg),
             "Page Fault at 0x%08x (%s, %s, %s%s%s)",
             faulting_address,
             present ? "page-protection" : "not-present",
             rw ? "write" : "read",
             user ? "user" : "kernel",
             reserved ? ", reserved bit overwritten" : "",
             id ? ", instruction fetch" : "");

    kernel_panic_registers(msg, regs);
}

void vmm_init(void) {
    register_interrupt_handler(14, page_fault_handler);

    // Clear kernel page directory
    memset(&kernel_page_directory, 0, sizeof(kernel_page_directory));

    // Identity map first 32MB (8 tables * 1024 entries * 4KB = 32MB)
    for (int t = 0; t < 8; t++) {
        for (int p = 0; p < 1024; p++) {
            uint32_t phys_addr = (t * 1024 + p) * PAGE_SIZE;
            identity_page_tables[t][p] = phys_addr | PAGE_PRESENT | PAGE_WRITE;
        }
        // Link table into directory
        kernel_page_directory[t] = ((uint32_t)&identity_page_tables[t]) | PAGE_PRESENT | PAGE_WRITE;
    }

    // Switch to page directory and enable paging in CR0
    vmm_switch_directory(&kernel_page_directory);

    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000001; // Enable PG (bit 31) and PE (bit 0)
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
}

void vmm_switch_directory(page_directory_t *dir) {
    __asm__ volatile("mov %0, %%cr3" : : "r"(dir));
}

page_directory_t *vmm_get_kernel_directory(void) {
    return &kernel_page_directory;
}

void vmm_map_page(uint32_t virt, uint32_t phys, uint32_t flags) {
    uint32_t pd_index = virt >> 22;
    uint32_t pt_index = (virt >> 12) & 0x03FF;

    if (!(kernel_page_directory[pd_index] & PAGE_PRESENT)) {
        // Allocate a new page table frame from PMM
        void *pt_frame = pmm_alloc_block();
        if (!pt_frame) return;
        memset(pt_frame, 0, PAGE_SIZE);
        kernel_page_directory[pd_index] = ((uint32_t)pt_frame) | PAGE_PRESENT | PAGE_WRITE | (flags & PAGE_USER);
    }

    page_table_t *pt = (page_table_t *)(kernel_page_directory[pd_index] & ~0xFFF);
    (*pt)[pt_index] = (phys & ~0xFFF) | (flags & 0xFFF) | PAGE_PRESENT;

    // Invalidate TLB for this virtual address
    __asm__ volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

void vmm_unmap_page(uint32_t virt) {
    uint32_t pd_index = virt >> 22;
    uint32_t pt_index = (virt >> 12) & 0x03FF;

    if (kernel_page_directory[pd_index] & PAGE_PRESENT) {
        page_table_t *pt = (page_table_t *)(kernel_page_directory[pd_index] & ~0xFFF);
        (*pt)[pt_index] = 0;
        __asm__ volatile("invlpg (%0)" : : "r"(virt) : "memory");
    }
}
