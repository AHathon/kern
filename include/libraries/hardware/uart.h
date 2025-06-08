#pragma once

#include "libraries/types.h"
#include "libraries/hardware/gpio.h"
#include "libraries/hardware/debug.h"
#include "libraries/hardware/mmio.h"
#include "libraries/hardware/mmio_vars.h"

#define UART_CLOCK 48000000

#define BAUDRATE 115200

void UART0_Init();
char uart0_getc();
void uart0_send(unsigned int c);
void uart0_puts(char *str);

void UART1_Init();
char uart1_getc();
void uart1_send(unsigned int c);
void uart1_puts(char *str);