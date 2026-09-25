#include "musl_compat.h"
#include "../../lib/stdio.h"

void musl_compat_init(void) {
    linux_translator_init();
}

int64_t musl_syscall(int64_t n, int64_t a1, int64_t a2, int64_t a3, int64_t a4, int64_t a5, int64_t a6) {
    return linux_syscall_translate(n, a1, a2, a3, a4, a5, a6);
}
