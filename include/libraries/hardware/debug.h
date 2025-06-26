#pragma once

#include "libraries/types.h"
#include "libraries/hardware/uart.h"
#include "libraries/ui/terminal.h"

#define LOG(...)  kprintf(__VA_ARGS__)
#define LOGT(...)  Terminal_Write(__VA_ARGS__)
                
#define ERROR(...)  (kprintf("ERROR: "),  \
                    kprintf(__VA_ARGS__))

size_t vasprintf(char * buf, const char *fmt, va_list args);
void panic();
void kprintf(char *fmt, ...);
void kHexDump(uint8_t *buf, size_t size);