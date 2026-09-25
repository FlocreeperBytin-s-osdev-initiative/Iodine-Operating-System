#include "serial.h"
#include "../arch/i386/io.h"

void serial_init(void) {
    outb(COM1_PORT + 1, 0x00);    // Disable all interrupts
    outb(COM1_PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(COM1_PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(COM1_PORT + 1, 0x00);    //                  (hi byte)
    outb(COM1_PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1_PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(COM1_PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int serial_received(void) {
    return inb(COM1_PORT + 5) & 1;
}

char serial_getc(void) {
    while (serial_received() == 0);
    return inb(COM1_PORT);
}

int serial_is_transmit_empty(void) {
    return inb(COM1_PORT + 5) & 0x20;
}

void serial_putc(char c) {
    while (serial_is_transmit_empty() == 0);
    outb(COM1_PORT, c);
    if (c == '\n') {
        while (serial_is_transmit_empty() == 0);
        outb(COM1_PORT, '\r');
    }
}

void serial_puts(const char *str) {
    if (!str) return;
    while (*str) {
        serial_putc(*str++);
    }
}
