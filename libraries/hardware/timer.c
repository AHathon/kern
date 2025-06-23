#include "libraries/hardware/timer.h"
#include "libraries/hardware/mmio_vars.h"
#include "libraries/hardware/irq.h"
#include "libraries/hardware/debug.h"

#define Hz 100

static uint64_t clkTicks = 0;

void resetSystemTimers()
{
    uint32_t now = *(volatile uint32_t *)(SYS_TIMER_CLO);

    // Setup timer 1 (IRQ 97)
    *(volatile uint32_t *)(SYS_TIMER_C1) = now + SCHEDULE_TIMER_INTERVAL;
    *(volatile uint32_t *)(SYS_TIMER_CS) = (1 << 1);

    // Setup timer 3 (IRQ 99)
    *(volatile uint32_t *)(SYS_TIMER_C3) = now + SCHEDULE_TIMER_INTERVAL;
    *(volatile uint32_t *)(SYS_TIMER_CS) = (1 << 3);
}

uint64_t GetCounterFreq()
{
    uint64_t freq;
    asm volatile("mrs %0, cntfrq_el0" : "=r" (freq));
    return freq;
}

void localTimerIrqReset()
{
    asm volatile("msr cntp_tval_el0, %0" :: "r"(clkTicks));
}

void localTimerIrqInit() 
{
    //Get irq interval
    uint64_t f = GetCounterFreq();
    clkTicks = f / Hz;

    //Reset timer
    localTimerIrqReset();

    //Enable local timer
    asm volatile("msr cntp_ctl_el0, %0" :: "r"(1L));

    //Unamsk irq
    enable_irq();
    LOGT("Initialized local timer with %d ticks per hit\n", clkTicks);
}