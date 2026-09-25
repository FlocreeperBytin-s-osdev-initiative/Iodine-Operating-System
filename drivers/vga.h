#ifndef DRIVERS_VGA_H
#define DRIVERS_VGA_H

#include <types.h>

#define VGA_WIDTH  80
#define VGA_HEIGHT 25

typedef enum {
    VGA_COLOR_BLACK         = 0,
    VGA_COLOR_BLUE          = 1,
    VGA_COLOR_GREEN         = 2,
    VGA_COLOR_CYAN          = 3,
    VGA_COLOR_RED           = 4,
    VGA_COLOR_MAGENTA       = 5,
    VGA_COLOR_BROWN         = 6,
    VGA_COLOR_LIGHT_GREY    = 7,
    VGA_COLOR_DARK_GREY     = 8,
    VGA_COLOR_LIGHT_BLUE    = 9,
    VGA_COLOR_LIGHT_GREEN   = 10,
    VGA_COLOR_LIGHT_CYAN    = 11,
    VGA_COLOR_LIGHT_RED     = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN   = 14,
    VGA_COLOR_YELLOW        = 14,
    VGA_COLOR_WHITE         = 15,
} vga_color_t;

void vga_init(void);
void vga_clear(void);
void vga_set_color(uint8_t fg, uint8_t bg);
uint8_t vga_get_color(void);
void vga_putc(char c);
void vga_puts(const char *str);
void vga_set_cursor(int x, int y);
void vga_get_cursor(int *x, int *y);
void vga_enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void vga_disable_cursor(void);
void vga_scroll(void);
void vga_put_entry_at(char c, uint8_t color, size_t x, size_t y);
char vga_get_char_at(size_t x, size_t y);
void vga_fill_rect(int x, int y, int w, int h, uint8_t color, char c);
void vga_draw_box(int x, int y, int w, int h, uint8_t fg, uint8_t bg, const char *title);

#endif /* DRIVERS_VGA_H */
