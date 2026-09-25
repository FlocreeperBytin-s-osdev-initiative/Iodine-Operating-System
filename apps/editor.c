#include "editor.h"
#include "../drivers/vga.h"
#include "../drivers/keyboard.h"
#include "../fs/vfs.h"
#include "../lib/string.h"
#include "../lib/stdio.h"
#include "../lib/stdlib.h"

#define EDIT_MAX_CHARS 4096
#define EDIT_ROWS 23
#define EDIT_COLS 80

void app_editor(const char *filename) {
    if (!filename || strlen(filename) == 0) {
        printf("Usage: edit <filename>\n");
        return;
    }

    char buffer[EDIT_MAX_CHARS];
    memset(buffer, 0, sizeof(buffer));
    int buf_len = 0;

    // Load file if exists
    vfs_node_t *file = vfs_resolve_path(filename);
    if (file && (file->flags & FS_FILE)) {
        buf_len = vfs_read(file, 0, sizeof(buffer) - 1, (uint8_t *)buffer);
        buffer[buf_len] = '\0';
    } else {
        // Create new file if it doesn't exist
        vfs_create_file(filename);
        file = vfs_resolve_path(filename);
    }

    int cursor_pos = 0;
    bool running = true;
    bool modified = false;

    while (running) {
        // Redraw whole screen
        vga_fill_rect(0, 0, VGA_WIDTH, 1, 0x1F, ' '); // Top bar blue
        vga_set_cursor(1, 0);
        printf(" Iodine Edit 1.0 - %s %s", filename, modified ? "[Modified]" : "");

        vga_fill_rect(0, 1, VGA_WIDTH, EDIT_ROWS, 0x07, ' '); // Edit area black

        // Render buffer
        int r = 1;
        int c = 0;
        int cur_screen_x = 0;
        int cur_screen_y = 1;

        for (int i = 0; i < buf_len && r < EDIT_ROWS + 1; i++) {
            if (i == cursor_pos) {
                cur_screen_x = c;
                cur_screen_y = r;
            }

            if (buffer[i] == '\n') {
                r++;
                c = 0;
            } else {
                vga_put_entry_at(buffer[i], 0x07, c, r);
                c++;
                if (c >= VGA_WIDTH) {
                    c = 0;
                    r++;
                }
            }
        }
        if (cursor_pos == buf_len) {
            cur_screen_x = c;
            cur_screen_y = r;
        }

        // Bottom status bar
        vga_fill_rect(0, 24, VGA_WIDTH, 1, 0x30, ' '); // Cyan
        vga_set_cursor(1, 24);
        printf(" ^S: Save   ^Q: Quit   Length: %d chars   Pos: %d", buf_len, cursor_pos);

        vga_set_cursor(cur_screen_x, cur_screen_y);

        // Read key
        char key = keyboard_read_char();
        if (key == 17) { // Ctrl+Q
            running = false;
        } else if (key == 19) { // Ctrl+S
            if (file) {
                vfs_write(file, 0, buf_len, (const uint8_t *)buffer);
                modified = false;
            }
        } else if ((unsigned char)key == KEY_LEFT) {
            if (cursor_pos > 0) cursor_pos--;
        } else if ((unsigned char)key == KEY_RIGHT) {
            if (cursor_pos < buf_len) cursor_pos++;
        } else if ((unsigned char)key == KEY_UP) {
            if (cursor_pos >= VGA_WIDTH) cursor_pos -= VGA_WIDTH;
            else cursor_pos = 0;
        } else if ((unsigned char)key == KEY_DOWN) {
            if (cursor_pos + VGA_WIDTH <= buf_len) cursor_pos += VGA_WIDTH;
            else cursor_pos = buf_len;
        } else if (key == '\b') {
            if (cursor_pos > 0) {
                for (int i = cursor_pos - 1; i < buf_len - 1; i++) {
                    buffer[i] = buffer[i + 1];
                }
                buf_len--;
                buffer[buf_len] = '\0';
                cursor_pos--;
                modified = true;
            }
        } else if (key == '\n' || (key >= 32 && key <= 126)) {
            if (buf_len < EDIT_MAX_CHARS - 1) {
                for (int i = buf_len; i > cursor_pos; i--) {
                    buffer[i] = buffer[i - 1];
                }
                buffer[cursor_pos] = key;
                buf_len++;
                buffer[buf_len] = '\0';
                cursor_pos++;
                modified = true;
            }
        }
    }

    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_clear();
}
