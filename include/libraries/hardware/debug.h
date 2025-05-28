#pragma once

#include "libraries/types.h"
#include "libraries/hardware/uart.h"

void kprintf(char *fmt, ...);
void kstrcpy(char *dest, char *src);