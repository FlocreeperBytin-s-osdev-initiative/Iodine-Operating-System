#include "ish.h"
#include "commands.h"
#include "../drivers/vga.h"
#include "../drivers/keyboard.h"
#include "../drivers/serial.h"
#include "../fs/vfs.h"
#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../lib/ctype.h"

#define MAX_LINE 256
#define MAX_ARGS 32
#define HISTORY_MAX 16

static char history[HISTORY_MAX][MAX_LINE];
static int history_count = 0;
static int history_index = 0;

static void print_prompt(void) {
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    printf("iodine:");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    printf("%s", shell_get_cwd());
    vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    printf("# ");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

static void add_history(const char *cmd) {
    if (!cmd || strlen(cmd) == 0) return;
    if (history_count > 0 && strcmp(history[(history_count - 1) % HISTORY_MAX], cmd) == 0) {
        return;
    }

    strncpy(history[history_count % HISTORY_MAX], cmd, MAX_LINE - 1);
    history_count++;
}

static void handle_tab_completion(char *line, int *len, int *pos) {
    int cmd_count = 0;
    const shell_command_t *cmds = commands_get_all(&cmd_count);

    // If first word, complete command name
    char *space = strchr(line, ' ');
    if (!space) {
        int matches = 0;
        const char *last_match = NULL;

        for (int i = 0; i < cmd_count; i++) {
            if (strncmp(cmds[i].name, line, *len) == 0) {
                matches++;
                last_match = cmds[i].name;
            }
        }

        if (matches == 1 && last_match) {
            strcpy(line, last_match);
            strcat(line, " ");
            *len = strlen(line);
            *pos = *len;

            // Redraw line
            vga_clear();
            print_prompt();
            printf("%s", line);
        } else if (matches > 1) {
            printf("\n");
            for (int i = 0; i < cmd_count; i++) {
                if (strncmp(cmds[i].name, line, *len) == 0) {
                    printf("%s  ", cmds[i].name);
                }
            }
            printf("\n");
            print_prompt();
            printf("%s", line);
        }
    }
}

void shell_main(void) {
    printf("\n");
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    printf("  ===============================================================\n");
    printf("          Welcome to Iodine Operating System v1.0.0              \n");
    printf("     Independent x86_32 Kernel | Multitasking & VFS Active       \n");
    printf("  ===============================================================\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    printf("  Type 'help' for command reference or 'fetch' for system specs.\n");
    printf("  Type 'snake' to play Snake, or 'edit <file>' to edit text.\n\n");

    char line[MAX_LINE];
    char *argv[MAX_ARGS];

    while (1) {
        print_prompt();
        memset(line, 0, sizeof(line));
        int len = 0;
        int pos = 0;
        history_index = history_count;

        while (1) {
            char c = keyboard_read_char();

            // Enter key
            if (c == '\n' || c == '\r') {
                putchar('\n');
                line[len] = '\0';
                break;
            }
            // Backspace
            else if (c == '\b') {
                if (pos > 0) {
                    for (int i = pos - 1; i < len - 1; i++) {
                        line[i] = line[i + 1];
                    }
                    len--;
                    pos--;
                    line[len] = '\0';

                    // Redraw rest of line
                    putchar('\b');
                    for (int i = pos; i < len; i++) {
                        putchar(line[i]);
                    }
                    putchar(' ');
                    // Move cursor back
                    for (int i = pos; i <= len; i++) {
                        putchar('\b');
                    }
                }
            }
            // Tab completion
            else if (c == '\t') {
                handle_tab_completion(line, &len, &pos);
            }
            // Ctrl+L: Clear screen
            else if (c == 12) {
                vga_clear();
                print_prompt();
                printf("%s", line);
            }
            // Ctrl+C: Cancel line
            else if (c == 3) {
                printf("^C\n");
                line[0] = '\0';
                len = 0;
                pos = 0;
                print_prompt();
            }
            // Arrow Up: History Previous
            else if ((unsigned char)c == KEY_UP) {
                if (history_count > 0 && history_index > 0) {
                    history_index--;
                    const char *hist_cmd = history[history_index % HISTORY_MAX];

                    // Clear currently displayed line
                    while (pos > 0) {
                        putchar('\b');
                        pos--;
                    }
                    for (int i = 0; i < len; i++) {
                        putchar(' ');
                    }
                    for (int i = 0; i < len; i++) {
                        putchar('\b');
                    }

                    strcpy(line, hist_cmd);
                    len = strlen(line);
                    pos = len;
                    printf("%s", line);
                }
            }
            // Arrow Down: History Next
            else if ((unsigned char)c == KEY_DOWN) {
                if (history_index < history_count - 1) {
                    history_index++;
                    const char *hist_cmd = history[history_index % HISTORY_MAX];

                    while (pos > 0) {
                        putchar('\b');
                        pos--;
                    }
                    for (int i = 0; i < len; i++) {
                        putchar(' ');
                    }
                    for (int i = 0; i < len; i++) {
                        putchar('\b');
                    }

                    strcpy(line, hist_cmd);
                    len = strlen(line);
                    pos = len;
                    printf("%s", line);
                } else if (history_index == history_count - 1) {
                    history_index = history_count;
                    while (pos > 0) {
                        putchar('\b');
                        pos--;
                    }
                    for (int i = 0; i < len; i++) {
                        putchar(' ');
                    }
                    for (int i = 0; i < len; i++) {
                        putchar('\b');
                    }
                    line[0] = '\0';
                    len = 0;
                    pos = 0;
                }
            }
            // Arrow Left
            else if ((unsigned char)c == KEY_LEFT) {
                if (pos > 0) {
                    pos--;
                    int x, y;
                    vga_get_cursor(&x, &y);
                    vga_set_cursor(x - 1, y);
                }
            }
            // Arrow Right
            else if ((unsigned char)c == KEY_RIGHT) {
                if (pos < len) {
                    pos++;
                    int x, y;
                    vga_get_cursor(&x, &y);
                    vga_set_cursor(x + 1, y);
                }
            }
            // Printable characters
            else if (isprint(c) && len < MAX_LINE - 1) {
                if (pos == len) {
                    line[len++] = c;
                    line[len] = '\0';
                    pos++;
                    putchar(c);
                } else {
                    for (int i = len; i > pos; i--) {
                        line[i] = line[i - 1];
                    }
                    line[pos] = c;
                    len++;
                    line[len] = '\0';
                    for (int i = pos; i < len; i++) {
                        putchar(line[i]);
                    }
                    pos++;
                    for (int i = pos; i < len; i++) {
                        putchar('\b');
                    }
                }
            }
        }

        // Add to history
        if (len > 0) {
            add_history(line);
        }

        // Parse arguments
        int argc = 0;
        char *p = line;
        while (*p && isspace(*p)) p++;

        while (*p && argc < MAX_ARGS - 1) {
            argv[argc++] = p;
            while (*p && !isspace(*p)) p++;
            if (*p) {
                *p++ = '\0';
                while (*p && isspace(*p)) p++;
            }
        }
        argv[argc] = NULL;

        if (argc > 0) {
            command_execute(argc, argv);
        }
    }
}
