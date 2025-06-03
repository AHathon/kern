#pragma once

#include "libraries/types.h"
#include "libraries/hardware/uart.h"

void kprintf(char *fmt, ...);
void kHexDump(uint8_t *buf, size_t size);
void kstrcpy(uint8_t *dest, uint8_t *src);
void kmemcpy(uint8_t *dest, uint8_t *src, size_t size);
void kmemset(uint8_t *src, size_t size);