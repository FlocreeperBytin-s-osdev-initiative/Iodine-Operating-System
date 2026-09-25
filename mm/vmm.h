#ifndef MM_VMM_H
#define MM_VMM_H

#include <types.h>
#include "../arch/i386/isr.h"

#define PAGE_PRESENT  0x01
#define PAGE_WRITE    0x02
#define PAGE_USER     0x04

typedef uint32_t page_directory_t[1024];
typedef uint32_t page_table_t[1024];

void vmm_init(void);
void vmm_map_page(uint32_t virt, uint32_t phys, uint32_t flags);
void vmm_unmap_page(uint32_t virt);
void vmm_switch_directory(page_directory_t *dir);
page_directory_t *vmm_get_kernel_directory(void);

#endif /* MM_VMM_H */
