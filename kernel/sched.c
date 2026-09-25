#include "sched.h"
#include "../drivers/timer.h"
#include "../arch/i386/io.h"

static bool scheduler_enabled = false;
static uint32_t ticks_in_slice = 0;
#define TIME_QUANTUM 5 // 5 ticks = 50ms

void sched_init(void) {
    scheduler_enabled = true;
}

void sched_enable(void) {
    scheduler_enabled = true;
}

void sched_disable(void) {
    scheduler_enabled = false;
}

static process_t *pick_next_task(process_t *curr) {
    process_t *proc_list = process_get_list();
    if (!proc_list) return NULL;

    process_t *p = curr ? curr->next : proc_list;
    if (!p) p = proc_list;

    process_t *start = p;
    do {
        if (p->state == PROCESS_STATE_READY) {
            return p;
        }
        p = p->next;
        if (!p) p = proc_list;
    } while (p != start);

    // If current is still running/ready, return it
    if (curr && (curr->state == PROCESS_STATE_RUNNING || curr->state == PROCESS_STATE_READY)) {
        return curr;
    }

    // Default to idle task (head of list, pid 0)
    return proc_list;
}

void sched_timer_tick(registers_t *regs) {
    (void)regs;
    if (!scheduler_enabled) return;

    process_t *curr = process_get_current();
    if (curr) {
        curr->cpu_ticks++;
    }

    uint32_t now = timer_get_ticks();
    process_t *p = process_get_list();
    while (p) {
        if (p->state == PROCESS_STATE_SLEEPING && now >= p->sleep_until_ticks) {
            p->state = PROCESS_STATE_READY;
        }
        p = p->next;
    }

    ticks_in_slice++;
    if (ticks_in_slice < TIME_QUANTUM) {
        return;
    }
    ticks_in_slice = 0;

    sched_yield();
}

void sched_yield(void) {
    if (!scheduler_enabled) return;

    process_t *curr = process_get_current();
    process_t *next = pick_next_task(curr);

    if (!next || next == curr) {
        return;
    }

    if (curr && curr->state == PROCESS_STATE_RUNNING) {
        curr->state = PROCESS_STATE_READY;
    }
    next->state = PROCESS_STATE_RUNNING;
    process_set_current(next);

    switch_task(&curr->esp, next->esp);
}
