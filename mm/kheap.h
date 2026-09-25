#ifndef MM_KHEAP_H
#define MM_KHEAP_H

#include <types.h>

#define KHEAP_START 0x01000000 // 16 MB mark
#define KHEAP_SIZE  0x01000000 // 16 MB size

void kheap_init(void);
void *kmalloc(size_t size);
void kfree(void *ptr);
void *kcalloc(size_t num, size_t size);
void *krealloc(void *ptr, size_t new_size);
void kheap_get_stats(size_t *used_bytes, size_t *free_bytes);

#endif /* MM_KHEAP_H */
