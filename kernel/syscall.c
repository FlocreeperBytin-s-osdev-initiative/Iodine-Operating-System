#include "syscall.h"
#include "process.h"
#include "sched.h"
#include "../drivers/vga.h"
#include "../drivers/speaker.h"
#include "../drivers/timer.h"
#include "../drivers/rtc.h"
#include "../drivers/power.h"
#include "../fs/vfs.h"
#include "../mm/kheap.h"
#include "../lib/stdio.h"

static void syscall_handler(registers_t *regs) {
    uint32_t syscall_num = regs->eax;
    uint32_t arg1 = regs->ebx;
    uint32_t arg2 = regs->ecx;
    uint32_t arg3 = regs->edx;

    switch (syscall_num) {
        case SYS_EXIT:
            process_exit((int)arg1);
            regs->eax = 0;
            break;

        case SYS_WRITE: {
            // arg1: fd, arg2: buf, arg3: count
            int fd = (int)arg1;
            const char *buf = (const char *)arg2;
            size_t count = (size_t)arg3;
            if (fd == 1 || fd == 2) { // stdout or stderr
                for (size_t i = 0; i < count; i++) {
                    putchar(buf[i]);
                }
                regs->eax = count;
            } else {
                regs->eax = 0;
            }
            break;
        }

        case SYS_GETPID: {
            process_t *curr = process_get_current();
            regs->eax = curr ? curr->pid : 0;
            break;
        }

        case SYS_YIELD:
            sched_yield();
            regs->eax = 0;
            break;

        case SYS_SLEEP:
            process_sleep(arg1);
            regs->eax = 0;
            break;

        case SYS_MALLOC:
            regs->eax = (uint32_t)kmalloc(arg1);
            break;

        case SYS_FREE:
            kfree((void *)arg1);
            regs->eax = 0;
            break;

        case SYS_CLEAR:
            vga_clear();
            regs->eax = 0;
            break;

        case SYS_UPTIME:
            regs->eax = timer_get_uptime_seconds();
            break;

        case SYS_REBOOT:
            power_reboot();
            regs->eax = 0;
            break;

        case SYS_BEEP:
            speaker_beep(arg1, arg2);
            regs->eax = 0;
            break;

        default:
            regs->eax = (uint32_t)-1;
            break;
    }
}

void syscall_init(void) {
    register_interrupt_handler(128, syscall_handler);
}
