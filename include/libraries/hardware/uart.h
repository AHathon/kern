#pragma once

#include "libraries/types.h"
#include "libraries/hardware/debug.h"
#include "libraries/hardware/mmio.h"
#include "libraries/hardware/mmio_vars.h"

void UART1_Init();

char uart1_getc();
void uart1_send(unsigned int c);
void uart1_puts(char *str);