#include "libraries/hardware/gpio.h"

void GPIO_SetAlt(unsigned int gpio, unsigned int alt) 
{
    unsigned int reg = gpio / 10;
    unsigned int shift = (gpio % 10) * 3;

    volatile unsigned int* gpfsel = GPFSEL0 + reg;
    *gpfsel &= ~(0b111 << shift);       // Clear bits
    *gpfsel |=  (alt    << shift);      // Set ALT function
}