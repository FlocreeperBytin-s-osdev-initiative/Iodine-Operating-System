#include "stdlib.h"
#include "ctype.h"
#include "string.h"

static unsigned long int next_rand = 1;

int rand(void) {
    next_rand = next_rand * 1103515245 + 12345;
    return (unsigned int)(next_rand / 65536) % 32768;
}

void srand(unsigned int seed) {
    next_rand = seed;
}

int abs(int n) {
    return (n < 0) ? -n : n;
}

int atoi(const char *str) {
    if (!str) return 0;
    while (isspace(*str)) str++;
    int sign = 1;
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    int res = 0;
    while (isdigit(*str)) {
        res = res * 10 + (*str - '0');
        str++;
    }
    return res * sign;
}

long strtol(const char *str, char **endptr, int base) {
    if (!str) return 0;
    const char *s = str;
    while (isspace(*s)) s++;
    int sign = 1;
    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    if ((base == 0 || base == 16) && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        s += 2;
        base = 16;
    } else if (base == 0) {
        if (*s == '0') base = 8;
        else base = 10;
    }

    long val = 0;
    while (*s) {
        int digit;
        if (isdigit(*s)) digit = *s - '0';
        else if (isalpha(*s)) digit = tolower(*s) - 'a' + 10;
        else break;

        if (digit >= base) break;
        val = val * base + digit;
        s++;
    }

    if (endptr) *endptr = (char *)s;
    return val * sign;
}

char *itoa(int value, char *str, int base) {
    if (!str || base < 2 || base > 36) return str;
    char *ptr = str;
    char *ptr1 = str;
    char tmp_char;
    int tmp_value;

    if (value < 0 && base == 10) {
        *ptr++ = '-';
        str++;
        ptr1++;
        value = -value;
    }

    do {
        tmp_value = value;
        value /= base;
        int rem = tmp_value - value * base;
        if (rem < 0) rem = -rem;
        *ptr++ = "0123456789abcdefghijklmnopqrstuvwxyz"[rem];
    } while (value);

    *ptr-- = '\0';
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
    return str;
}

char *uitoa(unsigned int value, char *str, int base) {
    if (!str || base < 2 || base > 36) return str;
    char *ptr = str;
    char *ptr1 = str;
    char tmp_char;

    do {
        unsigned int rem = value % base;
        *ptr++ = "0123456789abcdefghijklmnopqrstuvwxyz"[rem];
        value /= base;
    } while (value);

    *ptr-- = '\0';
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
    return str;
}
