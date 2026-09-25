#ifndef COMPAT_MUSL_COMPAT_H
#define COMPAT_MUSL_COMPAT_H

#include <types.h>
#include "../linux/linux_sys.h"

int64_t musl_syscall(int64_t n, int64_t a1, int64_t a2, int64_t a3, int64_t a4, int64_t a5, int64_t a6);
void musl_compat_init(void);

#endif /* COMPAT_MUSL_COMPAT_H */
