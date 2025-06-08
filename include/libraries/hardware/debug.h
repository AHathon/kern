#pragma once

#include "libraries/types.h"
#include "libraries/hardware/uart.h"
#include "libraries/ui/terminal.h"

#define LOG(...)                              \
                (Terminal_Write(__VA_ARGS__), \
                kprintf(__VA_ARGS__))
                
#define ERROR(...)  (kprintf("ERROR: "),  \
                    kprintf(__VA_ARGS__))

#define ASSERT(exp)                 \
    (void) ((exp) || (              \
        LOG("Assert: %s\n", #exp),  \
        panic(),                    \
        0                           \
    ))                              \


size_t vasprintf(char * buf, const char *fmt, va_list args);
void panic();
void kprintf(char *fmt, ...);
void kHexDump(uint8_t *buf, size_t size);
void kstrcpy(char *dest, const char *src);
void kmemcpy(uint8_t *dest, uint8_t *src, size_t size);
void kmemset(uint8_t *src, size_t size);
size_t kstrlen(const char *str);