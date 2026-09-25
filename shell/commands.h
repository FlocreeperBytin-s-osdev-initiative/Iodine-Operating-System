#ifndef SHELL_COMMANDS_H
#define SHELL_COMMANDS_H

#include <types.h>

typedef void (*cmd_func_t)(int argc, char **argv);

typedef struct {
    const char *name;
    const char *description;
    const char *usage;
    cmd_func_t func;
} shell_command_t;

void commands_init(void);
void command_execute(int argc, char **argv);
const shell_command_t *commands_get_all(int *count);
const char *shell_get_cwd(void);
void shell_set_cwd(const char *path);

#endif /* SHELL_COMMANDS_H */
