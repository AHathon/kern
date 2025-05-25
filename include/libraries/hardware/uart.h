#pragma once

#include "libraries/types.h"
#include "libraries/hardware/gpio.h"
#include "libraries/hardware/debug.h"
#include "libraries/hardware/mapping.h"

#define UART1_ENABLE    ((volatile unsigned int *)(UART_BASE+0x5004))
#define UART1_IO        ((volatile unsigned int *)(UART_BASE+0x5040))
#define UART1_IER       ((volatile unsigned int *)(UART_BASE+0x5044))
#define UART1_IIR       ((volatile unsigned int *)(UART_BASE+0x5048))
#define UART1_LCR       ((volatile unsigned int *)(UART_BASE+0x504C))
#define UART1_MCR       ((volatile unsigned int *)(UART_BASE+0x5050))
#define UART1_LSR       ((volatile unsigned int *)(UART_BASE+0x5054))
#define UART1_MSR       ((volatile unsigned int *)(UART_BASE+0x5058))
#define UART1_SCRATCH   ((volatile unsigned int *)(UART_BASE+0x505C))
#define UART1_CNTL      ((volatile unsigned int *)(UART_BASE+0x5060))
#define UART1_STAT      ((volatile unsigned int *)(UART_BASE+0x5064))
#define UART1_BAUD      ((volatile unsigned int *)(UART_BASE+0x5068))

void UART1_Init();

char uart1_getc();
void uart1_send(unsigned int c);
void uart1_puts(char *str);