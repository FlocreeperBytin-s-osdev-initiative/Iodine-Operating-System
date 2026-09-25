#include "kheap.h"
#include "../lib/string.h"
#include "../lib/stdio.h"

#define HEAP_MAGIC 0xCAFEBABE

typedef struct heap_block {
    uint32_t magic;
    uint32_t size;
    uint8_t  is_free;
    struct heap_block *next;
    struct heap_block *prev;
} heap_block_t;

#define ALIGN8(x) (((x) + 7) & ~7)

static heap_block_t *heap_head = NULL;

void kheap_init(void) {
    heap_head = (heap_block_t *)KHEAP_START;
    heap_head->magic = HEAP_MAGIC;
    heap_head->size = KHEAP_SIZE - sizeof(heap_block_t);
    heap_head->is_free = 1;
    heap_head->next = NULL;
    heap_head->prev = NULL;
}

void *kmalloc(size_t size) {
    if (size == 0) return NULL;
    size = ALIGN8(size);

    heap_block_t *curr = heap_head;
    while (curr) {
        if (curr->magic != HEAP_MAGIC) {
            printf("[KHEAP ERROR] Corrupt heap block header!\n");
            return NULL;
        }

        if (curr->is_free && curr->size >= size) {
            // Can we split this block?
            if (curr->size >= size + sizeof(heap_block_t) + 16) {
                heap_block_t *new_block = (heap_block_t *)((uint8_t *)curr + sizeof(heap_block_t) + size);
                new_block->magic = HEAP_MAGIC;
                new_block->size = curr->size - size - sizeof(heap_block_t);
                new_block->is_free = 1;
                new_block->next = curr->next;
                new_block->prev = curr;

                if (curr->next) {
                    curr->next->prev = new_block;
                }
                curr->next = new_block;
                curr->size = size;
            }

            curr->is_free = 0;
            return (void *)((uint8_t *)curr + sizeof(heap_block_t));
        }

        curr = curr->next;
    }

    printf("[KHEAP ERROR] Out of kernel heap memory!\n");
    return NULL;
}

void kfree(void *ptr) {
    if (!ptr) return;

    heap_block_t *block = (heap_block_t *)((uint8_t *)ptr - sizeof(heap_block_t));
    if (block->magic != HEAP_MAGIC) {
        printf("[KHEAP ERROR] Invalid pointer or heap corruption on kfree!\n");
        return;
    }

    block->is_free = 1;

    // Coalesce with next block if free
    if (block->next && block->next->is_free) {
        block->size += sizeof(heap_block_t) + block->next->size;
        block->next = block->next->next;
        if (block->next) {
            block->next->prev = block;
        }
    }

    // Coalesce with prev block if free
    if (block->prev && block->prev->is_free) {
        block->prev->size += sizeof(heap_block_t) + block->size;
        block->prev->next = block->next;
        if (block->next) {
            block->next->prev = block->prev;
        }
    }
}

void *kcalloc(size_t num, size_t size) {
    size_t total = num * size;
    void *ptr = kmalloc(total);
    if (ptr) {
        memset(ptr, 0, total);
    }
    return ptr;
}

void *krealloc(void *ptr, size_t new_size) {
    if (!ptr) return kmalloc(new_size);
    if (new_size == 0) {
        kfree(ptr);
        return NULL;
    }

    heap_block_t *block = (heap_block_t *)((uint8_t *)ptr - sizeof(heap_block_t));
    if (block->magic != HEAP_MAGIC) return NULL;

    if (block->size >= new_size) {
        return ptr;
    }

    void *new_ptr = kmalloc(new_size);
    if (!new_ptr) return NULL;

    memcpy(new_ptr, ptr, block->size);
    kfree(ptr);
    return new_ptr;
}

void kheap_get_stats(size_t *used_bytes, size_t *free_bytes) {
    size_t used = 0;
    size_t free = 0;

    heap_block_t *curr = heap_head;
    while (curr) {
        if (curr->magic != HEAP_MAGIC) break;
        if (curr->is_free) {
            free += curr->size;
        } else {
            used += curr->size + sizeof(heap_block_t);
        }
        curr = curr->next;
    }

    if (used_bytes) *used_bytes = used;
    if (free_bytes) *free_bytes = free;
}
