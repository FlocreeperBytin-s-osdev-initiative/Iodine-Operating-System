#ifndef KERNEL_PROCESS_H
#define KERNEL_PROCESS_H

#include <types.h>

typedef enum {
    PROCESS_STATE_READY = 0,
    PROCESS_STATE_RUNNING = 1,
    PROCESS_STATE_SLEEPING = 2,
    PROCESS_STATE_BLOCKED = 3,
    PROCESS_STATE_ZOMBIE = 4
} process_state_t;

typedef struct process {
    pid_t pid;
    char name[32];
    process_state_t state;
    uint32_t esp;
    uint32_t ebp;
    uint32_t eip;
    uint8_t *stack;
    uint32_t stack_size;
    uint32_t sleep_until_ticks;
    int priority;
    uint32_t cpu_ticks;
    struct process *next;
} process_t;

typedef struct {
    pid_t pid;
    char name[32];
    int state;
    int priority;
    uint32_t cpu_ticks;
} process_info_t;

void process_init(void);
process_t *process_create(const char *name, void (*entry_point)(void));
process_t *process_get_current(void);
void process_sleep(uint32_t ticks);
void process_exit(int code);
void process_yield(void);
int process_list_all(process_info_t *list, int max);
int process_kill(pid_t pid);

#endif /* KERNEL_PROCESS_H */
