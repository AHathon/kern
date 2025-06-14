#include "libraries/hardware/gpio.h"

void GPIO_SetAlt(uint32_t gpio, uint32_t alt) 
{
    uint32_t reg = gpio / 10;
    uint32_t shift = (gpio % 10) * 3;

    volatile uint32_t* gpfsel = GPFSEL(reg);
    *gpfsel &= ~(0b111 << shift);       // Clear bits
    *gpfsel |=  (alt    << shift);      // Set ALT function
}

void GPIO_SetPullNone(uint32_t gpio)
{
    *GPPUD = 0;
    for (volatile int r = 0; r < 150; r++) asm volatile("nop");

    if (gpio < 32)
        *GPPUDCLK0 = (1 << gpio);
    else
        *GPPUDCLK1 = (1 << (gpio - 32));

    for (volatile int r = 0; r < 150; r++) asm volatile("nop");

    if (gpio < 32)
        *GPPUDCLK0 = 0;
    else
        *GPPUDCLK1 = 0;
}

void GPIO_Write(uint32_t gpio, int32_t value)
{
    if (value)
        *GPSET(gpio / 32) = (1 << (gpio % 32));
    else
        *GPCLR(gpio / 32) = (1 << (gpio % 32));
}
