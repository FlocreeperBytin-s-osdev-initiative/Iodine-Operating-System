#include "vga.h"
#include "../arch/i386/io.h"
#include "../lib/string.h"

static volatile uint16_t *vga_buffer = (volatile uint16_t *)0xB8000;
static int vga_column = 0;
static int vga_row = 0;
static uint8_t vga_current_color = 0x07; // Light grey on black

static inline uint8_t vga_entry_color(uint8_t fg, uint8_t bg) {
    return fg | (bg << 4);
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t)uc | ((uint16_t)color << 8);
}

void vga_set_cursor(int x, int y) {
    if (x < 0) x = 0;
    if (x >= VGA_WIDTH) x = VGA_WIDTH - 1;
    if (y < 0) y = 0;
    if (y >= VGA_HEIGHT) y = VGA_HEIGHT - 1;

    vga_column = x;
    vga_row = y;

    uint16_t pos = y * VGA_WIDTH + x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void vga_get_cursor(int *x, int *y) {
    if (x) *x = vga_column;
    if (y) *y = vga_row;
}

void vga_enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);
    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

void vga_disable_cursor(void) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

void vga_set_color(uint8_t fg, uint8_t bg) {
    vga_current_color = vga_entry_color(fg, bg);
}

uint8_t vga_get_color(void) {
    return vga_current_color;
}

void vga_put_entry_at(char c, uint8_t color, size_t x, size_t y) {
    if (x < VGA_WIDTH && y < VGA_HEIGHT) {
        vga_buffer[y * VGA_WIDTH + x] = vga_entry((unsigned char)c, color);
    }
}

char vga_get_char_at(size_t x, size_t y) {
    if (x < VGA_WIDTH && y < VGA_HEIGHT) {
        return (char)(vga_buffer[y * VGA_WIDTH + x] & 0xFF);
    }
    return ' ';
}

void vga_scroll(void) {
    for (int y = 0; y < VGA_HEIGHT - 1; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    for (int x = 0; x < VGA_WIDTH; x++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', vga_current_color);
    }
    vga_row = VGA_HEIGHT - 1;
}

void vga_clear(void) {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_entry(' ', vga_current_color);
        }
    }
    vga_set_cursor(0, 0);
}

void vga_putc(char c) {
    if (c == '\n') {
        vga_column = 0;
        vga_row++;
    } else if (c == '\r') {
        vga_column = 0;
    } else if (c == '\b') {
        if (vga_column > 0) {
            vga_column--;
            vga_put_entry_at(' ', vga_current_color, vga_column, vga_row);
        } else if (vga_row > 0) {
            vga_row--;
            vga_column = VGA_WIDTH - 1;
            vga_put_entry_at(' ', vga_current_color, vga_column, vga_row);
        }
    } else if (c == '\t') {
        int tab_size = 4;
        int next_tab = (vga_column + tab_size) & ~(tab_size - 1);
        while (vga_column < next_tab && vga_column < VGA_WIDTH) {
            vga_put_entry_at(' ', vga_current_color, vga_column, vga_row);
            vga_column++;
        }
    } else if (c >= 32 || (unsigned char)c >= 128) {
        vga_put_entry_at(c, vga_current_color, vga_column, vga_row);
        vga_column++;
    }

    if (vga_column >= VGA_WIDTH) {
        vga_column = 0;
        vga_row++;
    }

    if (vga_row >= VGA_HEIGHT) {
        vga_scroll();
    }

    vga_set_cursor(vga_column, vga_row);
}

void vga_puts(const char *str) {
    if (!str) return;
    while (*str) {
        vga_putc(*str++);
    }
}

void vga_fill_rect(int x, int y, int w, int h, uint8_t color, char c) {
    for (int r = y; r < y + h && r < VGA_HEIGHT; r++) {
        for (int col = x; col < x + w && col < VGA_WIDTH; col++) {
            vga_put_entry_at(c, color, col, r);
        }
    }
}

void vga_draw_box(int x, int y, int w, int h, uint8_t fg, uint8_t bg, const char *title) {
    uint8_t color = vga_entry_color(fg, bg);

    // Fill inner
    vga_fill_rect(x + 1, y + 1, w - 2, h - 2, color, ' ');

    // Top and bottom borders
    for (int c = x; c < x + w; c++) {
        vga_put_entry_at('-', color, c, y);
        vga_put_entry_at('-', color, c, y + h - 1);
    }
    // Left and right borders
    for (int r = y; r < y + h; r++) {
        vga_put_entry_at('|', color, x, r);
        vga_put_entry_at('|', color, x + w - 1, r);
    }

    // Corners
    vga_put_entry_at('+', color, x, y);
    vga_put_entry_at('+', color, x + w - 1, y);
    vga_put_entry_at('+', color, x, y + h - 1);
    vga_put_entry_at('+', color, x + w - 1, y + h - 1);

    // Title
    if (title && w > 4) {
        int title_len = strlen(title);
        int title_x = x + (w - title_len) / 2;
        if (title_x < x + 1) title_x = x + 1;
        for (int i = 0; i < title_len && (title_x + i) < (x + w - 1); i++) {
            vga_put_entry_at(title[i], vga_entry_color(VGA_COLOR_YELLOW, bg), title_x + i, y);
        }
    }
}

void vga_init(void) {
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_enable_cursor(14, 15);
    vga_clear();
}
