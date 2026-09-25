#ifndef ARCH_X86_64_SYSCALL64_H
#define ARCH_X86_64_SYSCALL64_H

#include "types64.h"

#define MSR_EFER   0xC0000080
#define MSR_STAR   0xC0000081
#define MSR_LSTAR  0xC0000082
#define MSR_SFMASK 0xC0000084

#define EFER_SCE   (1ULL << 0)  // System Call Enable
#define EFER_LME   (1ULL << 8)  // Long Mode Enable
#define EFER_LMA   (1ULL << 10) // Long Mode Active
#define EFER_NXE   (1ULL << 11) // No-Execute Enable

/* 64-bit Saved Register Frame on Syscall/Interrupt */
typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t rip, cs, rflags, rsp, ss;
} registers64_t;

void syscall64_init(void);

#endif /* ARCH_X86_64_SYSCALL64_H */
