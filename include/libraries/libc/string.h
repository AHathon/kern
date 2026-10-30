#pragma once

#include "libraries/hardware/debug.h"
#include "libraries/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ASSERT(exp)                 \
    (void) ((exp) || (              \
        LOG("Assert: %s\n", #exp),  \
        panic(),                    \
        0                           \
    ))                              \

#define ALIGN(x) __attribute__((aligned(x)))

void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *dest, int c, size_t n);
int memcmp(const void *a, const void *b, size_t n);
void *memchr(const void *s, int c, size_t n);
size_t strlen(const char *s);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, size_t n);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
char *strstr(const char *haystack, const char *needle);

#ifdef __cplusplus
}
#endif
