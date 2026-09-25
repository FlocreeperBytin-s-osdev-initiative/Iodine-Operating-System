#include "matrix.h"
#include "../drivers/vga.h"
#include "../drivers/keyboard.h"
#include "../drivers/timer.h"
#include "../lib/stdlib.h"

#define MATRIX_COLS 80
#define MATRIX_ROWS 25

void app_matrix(void) {
    vga_clear();

    int drops[MATRIX_COLS];
    for (int i = 0; i < MATRIX_COLS; i++) {
        drops[i] = rand() % MATRIX_ROWS;
    }

    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789@#$%&*+-=";
    int charset_len = sizeof(charset) - 1;

    while (!keyboard_has_char()) {
        for (int x = 0; x < MATRIX_COLS; x += 2) { // Every 2 columns for nice spacing
            int y = drops[x];

            // Head character (bright white / light green)
            char c = charset[rand() % charset_len];
            if (y >= 0 && y < MATRIX_ROWS) {
                vga_put_entry_at(c, 0x0F, x, y); // Bright white head
            }

            // Trail 1 (light green)
            if (y - 1 >= 0 && y - 1 < MATRIX_ROWS) {
                char tc = charset[rand() % charset_len];
                vga_put_entry_at(tc, 0x0A, x, y - 1);
            }

            // Trail 2 (dark green)
            if (y - 4 >= 0 && y - 4 < MATRIX_ROWS) {
                char tc = charset[rand() % charset_len];
                vga_put_entry_at(tc, 0x02, x, y - 4);
            }

            // Trail 3 tail clear
            if (y - 8 >= 0 && y - 8 < MATRIX_ROWS) {
                vga_put_entry_at(' ', 0x07, x, y - 8);
            }

            drops[x]++;
            if (drops[x] - 8 >= MATRIX_ROWS && (rand() % 10 > 6)) {
                drops[x] = 0;
            }
        }

        timer_sleep(45);
    }

    keyboard_getc(); // consume pressed key
    vga_clear();
}
