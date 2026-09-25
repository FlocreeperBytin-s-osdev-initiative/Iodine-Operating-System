#ifndef STDLIB_H
#define STDLIB_H

#include <types.h>

void *kmalloc(size_t size);
void kfree(void *ptr);
void *kcalloc(size_t num, size_t size);
void *krealloc(void *ptr, size_t new_size);

int atoi(const char *str);
long strtol(const char *str, char **endptr, int base);
char *itoa(int value, char *str, int base);
char *uitoa(unsigned int value, char *str, int base);

int abs(int n);
int rand(void);
void srand(unsigned int seed);

#endif /* STDLIB_H */
