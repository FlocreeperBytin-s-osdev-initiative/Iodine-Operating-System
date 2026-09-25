#include "types64.h"
#include "syscall64.h"
#include "mmu.h"
#include "../../lib/stdio.h"
#include "../../compat/linux/linux_sys.h"

static inline void wrmsr64(uint32_t msr, uint64_t val) {
    uint32_t lo = (uint32_t)val;
    uint32_t hi = (uint32_t)(val >> 32);
    __asm__ volatile ("wrmsr" : : "c"(msr), "a"(lo), "d"(hi));
}

static inline uint64_t rdmsr64(uint32_t msr) {
    uint32_t lo, hi;
    __asm__ volatile ("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
    return ((uint64_t)hi << 32) | lo;
}

void syscall64_handler(registers64_t *regs) {
    // RAX = syscall number, RDI = arg1, RSI = arg2, RDX = arg3, R10 = arg4, R8 = arg5, R9 = arg6
    int64_t ret = linux_syscall_translate(regs->rax, regs->rdi, regs->rsi, regs->rdx, regs->r10, regs->r8, regs->r9);
    regs->rax = (uint64_t)ret;
}

void syscall64_init(void) {
    // Check if long mode is active before configuring 64-bit MSRs
    uint32_t edx = 0;
    __asm__ volatile (
        "mov $0x80000001, %%eax\n"
        "cpuid\n"
        : "=d"(edx)
        :
        : "eax", "ebx", "ecx"
    );

    if (edx & (1 << 29)) {
        // CPU supports Long Mode!
        uint64_t efer = rdmsr64(MSR_EFER);
        efer |= EFER_SCE; // Enable fast SYSCALL/SYSRET
        wrmsr64(MSR_EFER, efer);

        // STAR: User CS/SS in bits 63:48, Kernel CS/SS in bits 47:32
        uint64_t star = ((uint64_t)(0x1B | 3) << 48) | ((uint64_t)0x08 << 32);
        wrmsr64(MSR_STAR, star);

        // SFMASK: Clear IF (bit 9) on syscall
        wrmsr64(MSR_SFMASK, 0x200);
    }
}
