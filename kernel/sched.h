#ifndef KERNEL_SCHED_H
#define KERNEL_SCHED_H

#include <types.h>
#include "../arch/i386/isr.h"
#include "process.h"

void sched_init(void);
void sched_timer_tick(registers_t *regs);
void sched_yield(void);
void sched_enable(void);
void sched_disable(void);

extern void switch_task(uint32_t *prev_esp, uint32_t next_esp);
extern process_t *process_get_list(void);
extern void process_set_current(process_t *proc);

#endif /* KERNEL_SCHED_H */
