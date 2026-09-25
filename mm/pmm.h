#ifndef MM_PMM_H
#define MM_PMM_H

#include <types.h>

#define PAGE_SIZE 4096

void pmm_init(uint32_t mem_size_kb);
void *pmm_alloc_block(void);
void pmm_free_block(void *p);
uint32_t pmm_get_total_blocks(void);
uint32_t pmm_get_used_blocks(void);
uint32_t pmm_get_free_blocks(void);

#endif /* MM_PMM_H */
