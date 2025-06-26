#pragma once

#include "libraries/hardware/debug.h"
#include "libraries/types.h"

#define ASSERT(exp)                 \
    (void) ((exp) || (              \
        LOG("Assert: %s\n", #exp),  \
        panic(),                    \
        0                           \
    ))                              \

#define ALIGN(x) __attribute__((aligned(x)))

void strcpy(char *dest, const char *src);
void memcpy(uint8_t *dest, uint8_t *src, size_t size);
void memset(uint8_t *src, size_t size);
size_t strlen(const char *str);