#include "libraries/hardware/timer.h"
#include "libraries/hardware/mmio_vars.h"

#define TIMER_INTERVAL 1000000  // 1 million µs = 1s

void BCMTimerSetup()
{
    uint32_t now = *(volatile uint32_t *)(SYS_TIMER_CLO);

    // Setup timer 1 (IRQ 97)
    *(volatile uint32_t *)(SYS_TIMER_C1) = now + TIMER_INTERVAL;
    *(volatile uint32_t *)(SYS_TIMER_CS) |= (1 << 1);

    // Setup timer 3 (IRQ 99)
    *(volatile uint32_t *)(SYS_TIMER_C3) = now + TIMER_INTERVAL;
    *(volatile uint32_t *)(SYS_TIMER_CS) |= (1 << 3);
}