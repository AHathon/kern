#include "libraries/hardware/uart.h"

void UART0_Init() 
{
    //Disable UART0
    *UART0_CR = 0x00000000;

    //Clear pending interrupts
    *UART0_ICR = 0x7FF;

    //GPIO14/15 disable pull-up/down
    *GPPUD = 0;
    for (int r = 0; r < 150; r++) asm volatile("nop");

    *GPPUDCLK0 = (1 << 14) | (1 << 15);
    for (int r = 0; r < 150; r++) asm volatile("nop");

    *GPPUDCLK0 = 0;

    //Set baud rate (UART_CLOCK = 3 MHz for mini UART default)
    *UART0_IBRD = ((UART_CLOCK) / (16 * (BAUDRATE)));
    *UART0_FBRD = (((((UART_CLOCK) % (16 * (BAUDRATE))) * 64 + (BAUDRATE / 2)) / (BAUDRATE)));

    //Line control
    *UART0_LCRH = (1 << 4)     | //FEN (enable FIFOs)
                  (0b11 << 5);   //WLEN (8 bits)

    //Mask all UART interrupts
    *UART0_IMSC = (1 << 1) | 
                  (1 << 4) | 
                  (1 << 5) | 
                  (1 << 6) | 
                  (1 << 7) | 
                  (1 << 8) | 
                  (1 << 9) | 
                  (1 << 10);

    //Enable UART0, TX, RX
    *UART0_CR = (1 << 0) |  //Uart enable
                (1 << 8) |  //TX enable
                (1 << 9);   //RX enable

    //Map UART0 to GPIO
    GPIO_SetAlt(14, FUNC_A0);
    GPIO_SetAlt(15, FUNC_A0);
}

char uart0_getc() 
{
    while (*UART0_FR & UART_FR_RXFE);  // Wait until not empty
    char c = (char)(*UART0_DR);
    return (c == '\r') ? '\n' : c;
}

void uart0_send(unsigned int c) 
{
    while (*UART0_FR & UART_FR_TXFF);  // Wait until not full
    *UART0_DR = c;
}

void uart0_puts(char *str) 
{
    for (; *str; ++str) {
        if (*str == '\n') uart0_send('\r');
        uart0_send(*str);
    }
}

void UART1_Init() 
{
    register unsigned int r;

    //Initialize UART1
    *UART1_ENABLE |= 1;
    *UART1_CNTL = 0;
    *UART1_LCR = 3;
    *UART1_MCR = 0;
    *UART1_IER = 0;
    *UART1_IIR = 0xC6;
    *UART1_BAUD = ((UART_CLOCK) / (16 * (BAUDRATE)));

    *GPPUD = 0;
    r = 150; while (r--) { asm volatile("nop"); } 
    *GPPUDCLK0 = (1 << 14) | (1 << 15);
    r = 150; while (r--) { asm volatile("nop"); }
    *GPPUDCLK0 = 0;

    *UART1_CNTL = 3;

    //Map to gpio pins
    GPIO_SetAlt(14, FUNC_A5);   //(UART1 TXD)
    GPIO_SetAlt(15, FUNC_A5);   //(UART1 RXD)
}

char uart1_getc() 
{
    do { asm volatile("nop"); } while (!(*UART1_LSR & 0x01));

    char c = (char)(*UART1_IO);

    return (c == '\r') ? '\n' : c;
}

void uart1_send(unsigned int c) 
{
    do { asm volatile("nop"); } while (!(*UART1_LSR & 0x20));

    *UART1_IO = c;
}

void uart1_puts(char *str) 
{
    for (; *str; ++str) {
        if (*str == '\n')
            uart1_send('\r');
        uart1_send(*str);
    }
}