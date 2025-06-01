#pragma once

#include "libraries/types.h"
#include "libraries/hardware/debug.h"
#include "libraries/hardware/mmio.h"
#include "libraries/hardware/mmio_vars.h"

#define UART1_CLOCK 250000000
#define UART_BAUD ((UART1_CLOCK / (115200 * 8)) - 1)

void UART1_Init();

char uart1_getc();
void uart1_send(unsigned int c);
void uart1_puts(char *str);