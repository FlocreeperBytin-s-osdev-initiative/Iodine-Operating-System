#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "ctype.h"

extern void vga_putc(char c);
extern void serial_putc(char c);

void putchar(char c) {
    vga_putc(c);
    serial_putc(c);
}

void puts(const char *str) {
    if (!str) return;
    while (*str) {
        putchar(*str++);
    }
    putchar('\n');
}

static void buffer_putc(char **buf, size_t *rem, char c) {
    if (*rem > 1) {
        **buf = c;
        (*buf)++;
        (*rem)--;
    }
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap) {
    if (!str || size == 0) return 0;

    char *buf_ptr = str;
    size_t rem = size;
    int written = 0;

    const char *p = format;
    while (*p) {
        if (*p != '%') {
            buffer_putc(&buf_ptr, &rem, *p);
            written++;
            p++;
            continue;
        }

        p++; // skip '%'
        if (*p == '\0') break;

        // Flags
        bool pad_zero = false;
        bool left_align = false;
        if (*p == '0') {
            pad_zero = true;
            p++;
        } else if (*p == '-') {
            left_align = true;
            p++;
        }

        // Width
        int width = 0;
        while (isdigit(*p)) {
            width = width * 10 + (*p - '0');
            p++;
        }

        // Length modifiers (l, ll)
        int is_long = 0;
        if (*p == 'l') {
            is_long++;
            p++;
            if (*p == 'l') {
                is_long++;
                p++;
            }
        }

        // Specifier
        char spec = *p++;
        char tmp[64];
        char *arg_str = NULL;
        int arg_len = 0;

        switch (spec) {
            case 'c': {
                char c = (char)va_arg(ap, int);
                tmp[0] = c;
                tmp[1] = '\0';
                arg_str = tmp;
                arg_len = 1;
                break;
            }
            case 's': {
                arg_str = va_arg(ap, char *);
                if (!arg_str) arg_str = "(null)";
                arg_len = strlen(arg_str);
                break;
            }
            case 'd':
            case 'i': {
                if (is_long >= 2) {
                    long long val = va_arg(ap, long long);
                    if (val < 0) {
                        tmp[0] = '-';
                        uitoa((unsigned long long)-val, tmp + 1, 10);
                    } else {
                        uitoa((unsigned long long)val, tmp, 10);
                    }
                } else if (is_long == 1) {
                    long val = va_arg(ap, long);
                    itoa(val, tmp, 10);
                } else {
                    int val = va_arg(ap, int);
                    itoa(val, tmp, 10);
                }
                arg_str = tmp;
                arg_len = strlen(arg_str);
                break;
            }
            case 'u': {
                if (is_long >= 2) {
                    unsigned long long val = va_arg(ap, unsigned long long);
                    uitoa(val, tmp, 10);
                } else if (is_long == 1) {
                    unsigned long val = va_arg(ap, unsigned long);
                    uitoa(val, tmp, 10);
                } else {
                    unsigned int val = va_arg(ap, unsigned int);
                    uitoa(val, tmp, 10);
                }
                arg_str = tmp;
                arg_len = strlen(arg_str);
                break;
            }
            case 'x': {
                unsigned int val = va_arg(ap, unsigned int);
                uitoa(val, tmp, 16);
                arg_str = tmp;
                arg_len = strlen(arg_str);
                break;
            }
            case 'X': {
                unsigned int val = va_arg(ap, unsigned int);
                uitoa(val, tmp, 16);
                for (int i = 0; tmp[i]; i++) tmp[i] = toupper(tmp[i]);
                arg_str = tmp;
                arg_len = strlen(arg_str);
                break;
            }
            case 'p': {
                uintptr_t val = (uintptr_t)va_arg(ap, void *);
                tmp[0] = '0';
                tmp[1] = 'x';
                uitoa(val, tmp + 2, 16);
                arg_str = tmp;
                arg_len = strlen(arg_str);
                break;
            }
            case 'b': {
                unsigned int val = va_arg(ap, unsigned int);
                uitoa(val, tmp, 2);
                arg_str = tmp;
                arg_len = strlen(arg_str);
                break;
            }
            case '%': {
                tmp[0] = '%';
                tmp[1] = '\0';
                arg_str = tmp;
                arg_len = 1;
                break;
            }
            default:
                tmp[0] = '%';
                tmp[1] = spec;
                tmp[2] = '\0';
                arg_str = tmp;
                arg_len = 2;
                break;
        }

        // Padding
        int pad = width - arg_len;
        if (pad > 0 && !left_align) {
            char pad_char = pad_zero ? '0' : ' ';
            while (pad--) {
                buffer_putc(&buf_ptr, &rem, pad_char);
                written++;
            }
        }

        // Output string
        for (int i = 0; i < arg_len; i++) {
            buffer_putc(&buf_ptr, &rem, arg_str[i]);
            written++;
        }

        if (pad > 0 && left_align) {
            while (pad--) {
                buffer_putc(&buf_ptr, &rem, ' ');
                written++;
            }
        }
    }

    if (rem > 0) {
        *buf_ptr = '\0';
    } else {
        str[size - 1] = '\0';
    }

    return written;
}

int vsprintf(char *str, const char *format, va_list ap) {
    return vsnprintf(str, 0x7FFFFFFF, format, ap);
}

int sprintf(char *str, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int ret = vsprintf(str, format, ap);
    va_end(ap);
    return ret;
}

int snprintf(char *str, size_t size, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int ret = vsnprintf(str, size, format, ap);
    va_end(ap);
    return ret;
}

int printf(const char *format, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, format);
    int ret = vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);

    for (int i = 0; buf[i]; i++) {
        putchar(buf[i]);
    }
    return ret;
}
