#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

#include <types.h>
#include "../arch/i386/isr.h"

#define SYS_EXIT    0
#define SYS_FORK    1
#define SYS_READ    2
#define SYS_WRITE   3
#define SYS_OPEN    4
#define SYS_CLOSE   5
#define SYS_WAITPID 6
#define SYS_GETPID  7
#define SYS_YIELD   8
#define SYS_SLEEP   9
#define SYS_MALLOC  10
#define SYS_FREE    11
#define SYS_TIME    12
#define SYS_CLEAR   13
#define SYS_UPTIME  14
#define SYS_REBOOT  19
#define SYS_BEEP    20

void syscall_init(void);

#endif /* KERNEL_SYSCALL_H */
