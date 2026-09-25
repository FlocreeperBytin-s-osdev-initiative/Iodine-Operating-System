#ifndef DRIVERS_SERIAL_H
#define DRIVERS_SERIAL_H

#include <types.h>

#define COM1_PORT 0x3F8

void serial_init(void);
int serial_received(void);
char serial_getc(void);
int serial_is_transmit_empty(void);
void serial_putc(char c);
void serial_puts(const char *str);

#endif /* DRIVERS_SERIAL_H */
