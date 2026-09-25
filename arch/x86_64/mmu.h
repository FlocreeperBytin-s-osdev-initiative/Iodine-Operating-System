#ifndef ARCH_X86_64_MMU_H
#define ARCH_X86_64_MMU_H

#include "types64.h"

#define PAGE_SIZE_4K   4096
#define PAGE_SIZE_2M   (2 * 1024 * 1024)
#define PAGE_SIZE_1G   (1024 * 1024 * 1024)

#define PTE_PRESENT    (1ULL << 0)
#define PTE_WRITABLE   (1ULL << 1)
#define PTE_USER       (1ULL << 2)
#define PTE_HUGE       (1ULL << 7)
#define PTE_GLOBAL     (1ULL << 8)
#define PTE_NX         (1ULL << 63)

typedef uint64_t pml4e_t;
typedef uint64_t pdpte_t;
typedef uint64_t pde_t;
typedef uint64_t pte_t;

typedef struct {
    pml4e_t entries[512];
} __attribute__((aligned(4096))) pml4_table_t;

typedef struct {
    pdpte_t entries[512];
} __attribute__((aligned(4096))) pdpt_table_t;

typedef struct {
    pde_t entries[512];
} __attribute__((aligned(4096))) pd_table_t;

typedef struct {
    pte_t entries[512];
} __attribute__((aligned(4096))) pt_table_t;

#endif /* ARCH_X86_64_MMU_H */
