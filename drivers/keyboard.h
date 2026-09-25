#ifndef DRIVERS_KEYBOARD_H
#define DRIVERS_KEYBOARD_H

#include <types.h>

#define KEY_UP        0x80
#define KEY_DOWN      0x81
#define KEY_LEFT      0x82
#define KEY_RIGHT     0x83
#define KEY_HOME      0x84
#define KEY_END       0x85
#define KEY_DELETE    0x86
#define KEY_PAGE_UP   0x87
#define KEY_PAGE_DOWN 0x88

void keyboard_init(void);
bool keyboard_has_char(void);
char keyboard_getc(void);
char keyboard_read_char(void);
int keyboard_readline(char *buffer, int max_len);

#endif /* DRIVERS_KEYBOARD_H */
