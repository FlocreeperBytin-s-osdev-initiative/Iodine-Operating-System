#include "process.h"
#include "sched.h"
#include "../mm/kheap.h"
#include "../lib/string.h"
#include "../lib/stdio.h"
#include "../arch/i386/io.h"

#define STACK_SIZE 16384 // 16 KB stack per task

static process_t kernel_idle_proc;
static process_t *process_list = NULL;
static process_t *current_proc = NULL;
static pid_t next_pid = 0;

static void task_exit_wrapper(void) {
    process_exit(0);
}

void process_init(void) {
    // Initialize Task 0 (Kernel / Idle process)
    memset(&kernel_idle_proc, 0, sizeof(process_t));
    kernel_idle_proc.pid = next_pid++;
    strncpy(kernel_idle_proc.name, "idle", 31);
    kernel_idle_proc.state = PROCESS_STATE_RUNNING;
    kernel_idle_proc.priority = 1;
    kernel_idle_proc.next = NULL;

    process_list = &kernel_idle_proc;
    current_proc = &kernel_idle_proc;
}

process_t *process_create(const char *name, void (*entry_point)(void)) {
    process_t *proc = (process_t *)kmalloc(sizeof(process_t));
    if (!proc) return NULL;

    memset(proc, 0, sizeof(process_t));
    proc->pid = next_pid++;
    strncpy(proc->name, name, 31);
    proc->state = PROCESS_STATE_READY;
    proc->priority = 5;
    proc->stack_size = STACK_SIZE;
    proc->stack = (uint8_t *)kmalloc(STACK_SIZE);
    if (!proc->stack) {
        kfree(proc);
        return NULL;
    }

    // Set up top of stack (x86 stack grows downwards)
    uint32_t *sp = (uint32_t *)(proc->stack + STACK_SIZE);

    // Push task_exit_wrapper as return address if entry_point returns
    *(--sp) = (uint32_t)task_exit_wrapper;

    // Push entry point address
    *(--sp) = (uint32_t)entry_point;

    // Push initial registers matching switch_task layout:
    // eflags, ebp, ebx, esi, edi
    *(--sp) = 0x202; // EFLAGS: IF enabled
    *(--sp) = 0;     // EBP
    *(--sp) = 0;     // EBX
    *(--sp) = 0;     // ESI
    *(--sp) = 0;     // EDI

    proc->esp = (uint32_t)sp;

    // Append to process list
    cli();
    process_t *curr = process_list;
    while (curr->next) {
        curr = curr->next;
    }
    curr->next = proc;
    sti();

    return proc;
}

process_t *process_get_current(void) {
    return current_proc;
}

void process_set_current(process_t *proc) {
    current_proc = proc;
}

process_t *process_get_list(void) {
    return process_list;
}

void process_yield(void) {
    sched_yield();
}

void process_sleep(uint32_t ticks) {
    extern uint32_t timer_get_ticks(void);
    if (!current_proc) return;

    cli();
    current_proc->sleep_until_ticks = timer_get_ticks() + ticks;
    current_proc->state = PROCESS_STATE_SLEEPING;
    sti();

    sched_yield();
}

void process_exit(int code) {
    (void)code;
    cli();
    if (current_proc && current_proc->pid != 0) {
        current_proc->state = PROCESS_STATE_ZOMBIE;
    }
    sti();

    sched_yield();
    while (1) hlt();
}

int process_kill(pid_t pid) {
    if (pid <= 1) return -1; // Cannot kill idle or shell

    cli();
    process_t *curr = process_list;
    while (curr) {
        if (curr->pid == pid) {
            curr->state = PROCESS_STATE_ZOMBIE;
            sti();
            return 0;
        }
        curr = curr->next;
    }
    sti();
    return -1;
}

int process_list_all(process_info_t *list, int max) {
    if (!list || max <= 0) return 0;
    int count = 0;

    cli();
    process_t *curr = process_list;
    while (curr && count < max) {
        list[count].pid = curr->pid;
        strncpy(list[count].name, curr->name, 31);
        list[count].state = (int)curr->state;
        list[count].priority = curr->priority;
        list[count].cpu_ticks = curr->cpu_ticks;
        count++;
        curr = curr->next;
    }
    sti();

    return count;
}
