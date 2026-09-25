#include "pmm.h"
#include "../lib/string.h"
#include "../lib/stdio.h"

#define BLOCKS_PER_BYTE 8
#define MAX_PHYSICAL_MEM (128 * 1024 * 1024) // 128 MB default supported
#define MAX_BLOCKS (MAX_PHYSICAL_MEM / PAGE_SIZE) // 32768 blocks
#define BITMAP_SIZE (MAX_BLOCKS / BLOCKS_PER_BYTE) // 4096 bytes

// Bitmap placed in kernel memory
static uint8_t memory_bitmap[BITMAP_SIZE];
static uint32_t total_blocks = 0;
static uint32_t used_blocks = 0;

static inline void set_bit(uint32_t bit) {
    memory_bitmap[bit / 8] |= (1 << (bit % 8));
}

static inline void clear_bit(uint32_t bit) {
    memory_bitmap[bit / 8] &= ~(1 << (bit % 8));
}

static inline bool test_bit(uint32_t bit) {
    return (memory_bitmap[bit / 8] & (1 << (bit % 8))) != 0;
}

static int32_t find_first_free_block(void) {
    for (uint32_t i = 0; i < total_blocks / 8; i++) {
        if (memory_bitmap[i] != 0xFF) {
            for (int b = 0; b < 8; b++) {
                if (!(memory_bitmap[i] & (1 << b))) {
                    return i * 8 + b;
                }
            }
        }
    }
    return -1;
}

void pmm_init(uint32_t mem_size_kb) {
    if (mem_size_kb == 0) {
        mem_size_kb = 128 * 1024; // Default to 128MB
    }

    uint32_t total_bytes = mem_size_kb * 1024;
    if (total_bytes > MAX_PHYSICAL_MEM) {
        total_bytes = MAX_PHYSICAL_MEM;
    }

    total_blocks = total_bytes / PAGE_SIZE;
    used_blocks = total_blocks;

    // Initially mark everything as used
    memset(memory_bitmap, 0xFF, sizeof(memory_bitmap));

    // Free all blocks from 4MB up to total_blocks
    // First 4MB (1024 blocks) is reserved for kernel and hardware
    uint32_t kernel_reserved_blocks = (4 * 1024 * 1024) / PAGE_SIZE; // 1024 blocks
    for (uint32_t b = kernel_reserved_blocks; b < total_blocks; b++) {
        clear_bit(b);
        used_blocks--;
    }
}

void *pmm_alloc_block(void) {
    int32_t block = find_first_free_block();
    if (block == -1 || (uint32_t)block >= total_blocks) {
        return NULL; // Out of physical memory
    }

    set_bit(block);
    used_blocks++;

    return (void *)(block * PAGE_SIZE);
}

void pmm_free_block(void *p) {
    uint32_t addr = (uint32_t)p;
    uint32_t block = addr / PAGE_SIZE;

    if (block < total_blocks && test_bit(block)) {
        clear_bit(block);
        used_blocks--;
    }
}

uint32_t pmm_get_total_blocks(void) {
    return total_blocks;
}

uint32_t pmm_get_used_blocks(void) {
    return used_blocks;
}

uint32_t pmm_get_free_blocks(void) {
    return total_blocks - used_blocks;
}
