#include "snake.h"
#include "../drivers/vga.h"
#include "../drivers/keyboard.h"
#include "../drivers/timer.h"
#include "../drivers/speaker.h"
#include "../lib/stdio.h"
#include "../lib/stdlib.h"

#define BOARD_X 10
#define BOARD_Y 3
#define BOARD_W 60
#define BOARD_H 20
#define MAX_SNAKE_LEN 256

typedef struct {
    int x;
    int y;
} point_t;

void app_snake(void) {
    vga_clear();
    vga_draw_box(BOARD_X, BOARD_Y, BOARD_W, BOARD_H, VGA_COLOR_WHITE, VGA_COLOR_BLUE, " IODINE SNAKE ");

    vga_set_cursor(12, 1);
    printf("Controls: WASD or Arrow Keys | Q: Quit | Eat '*' to grow!");

    point_t snake[MAX_SNAKE_LEN];
    int length = 4;
    int dx = 1;
    int dy = 0;
    int score = 0;
    bool game_over = false;

    // Initial snake position
    int start_x = BOARD_X + BOARD_W / 2;
    int start_y = BOARD_Y + BOARD_H / 2;
    for (int i = 0; i < length; i++) {
        snake[i].x = start_x - i;
        snake[i].y = start_y;
        vga_put_entry_at('O', 0x1E, snake[i].x, snake[i].y); // Yellow on blue
    }

    // Place first food
    point_t food;
    food.x = BOARD_X + 2 + (rand() % (BOARD_W - 4));
    food.y = BOARD_Y + 2 + (rand() % (BOARD_H - 4));
    vga_put_entry_at('*', 0x1C, food.x, food.y); // Light red on blue

    while (!game_over) {
        // Update score display
        vga_set_cursor(BOARD_X + BOARD_W - 16, BOARD_Y);
        printf(" Score: %-4d ", score);

        // Check input
        while (keyboard_has_char()) {
            char c = keyboard_getc();
            if (c == 'q' || c == 'Q') {
                game_over = true;
                break;
            } else if ((c == 'w' || c == 'W' || (unsigned char)c == KEY_UP) && dy != 1) {
                dx = 0; dy = -1;
            } else if ((c == 's' || c == 'S' || (unsigned char)c == KEY_DOWN) && dy != -1) {
                dx = 0; dy = 1;
            } else if ((c == 'a' || c == 'A' || (unsigned char)c == KEY_LEFT) && dx != 1) {
                dx = -1; dy = 0;
            } else if ((c == 'd' || c == 'D' || (unsigned char)c == KEY_RIGHT) && dx != -1) {
                dx = 1; dy = 0;
            }
        }
        if (game_over) break;

        // Calculate new head
        point_t new_head;
        new_head.x = snake[0].x + dx;
        new_head.y = snake[0].y + dy;

        // Wall collision check
        if (new_head.x <= BOARD_X || new_head.x >= BOARD_X + BOARD_W - 1 ||
            new_head.y <= BOARD_Y || new_head.y >= BOARD_Y + BOARD_H - 1) {
            game_over = true;
            break;
        }

        // Self collision check
        for (int i = 0; i < length; i++) {
            if (snake[i].x == new_head.x && snake[i].y == new_head.y) {
                game_over = true;
                break;
            }
        }
        if (game_over) break;

        // Check if food eaten
        bool ate_food = (new_head.x == food.x && new_head.y == food.y);
        if (ate_food) {
            score += 10;
            speaker_beep(880, 30); // High beep
            if (length < MAX_SNAKE_LEN - 1) {
                length++;
            }
            // Spawn new food
            food.x = BOARD_X + 2 + (rand() % (BOARD_W - 4));
            food.y = BOARD_Y + 2 + (rand() % (BOARD_H - 4));
            vga_put_entry_at('*', 0x1C, food.x, food.y);
        } else {
            // Clear tail
            vga_put_entry_at(' ', 0x1F, snake[length - 1].x, snake[length - 1].y);
        }

        // Shift body
        for (int i = length - 1; i > 0; i--) {
            snake[i] = snake[i - 1];
        }
        snake[0] = new_head;

        // Draw snake
        vga_put_entry_at('@', 0x1A, snake[0].x, snake[0].y); // Head: Green on blue
        if (length > 1) {
            vga_put_entry_at('o', 0x1E, snake[1].x, snake[1].y); // Body: Yellow on blue
        }

        // Delay loop (speed increases slightly as score increases)
        int delay = 100 - (score / 10) * 3;
        if (delay < 40) delay = 40;
        timer_sleep(delay);
    }

    // Game Over Fanfare
    speaker_beep(220, 150);
    timer_sleep(50);
    speaker_beep(160, 300);

    vga_draw_box(BOARD_X + 15, BOARD_Y + 7, 30, 6, VGA_COLOR_WHITE, VGA_COLOR_RED, " GAME OVER ");
    vga_set_cursor(BOARD_X + 18, BOARD_Y + 9);
    printf("Final Score: %d", score);
    vga_set_cursor(BOARD_X + 17, BOARD_Y + 11);
    printf("Press Any Key to Exit");

    while (!keyboard_has_char()) {
        timer_sleep(50);
    }
    keyboard_getc();

    vga_clear();
}
