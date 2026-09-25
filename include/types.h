#ifndef TYPES_H
#define TYPES_H

typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

#ifdef __x86_64__
typedef uint64_t           size_t;
typedef int64_t            ssize_t;
typedef uint64_t           uintptr_t;
typedef int64_t            intptr_t;
#else
typedef uint32_t           size_t;
typedef int32_t            ssize_t;
typedef uint32_t           uintptr_t;
typedef int32_t            intptr_t;
#endif

typedef int32_t            pid_t;

#define NULL ((void *)0)

#ifndef __cplusplus
typedef enum {
    false = 0,
    true  = 1
} bool;
#endif

#endif /* TYPES_H */
