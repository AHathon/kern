#include "libraries/hardware/timer.h"
#include "libraries/hardware/mmio_vars.h"
#include "libraries/hardware/irq.h"

void resetSystemTimers()
{
    uint32_t now = *(volatile uint32_t *)(SYS_TIMER_CLO);

    // Setup timer 1 (IRQ 97)
    *(volatile uint32_t *)(SYS_TIMER_C1) = now + TIMER_INTERVAL;
    *(volatile uint32_t *)(SYS_TIMER_CS) = (1 << 1);

    // Setup timer 3 (IRQ 99)
    *(volatile uint32_t *)(SYS_TIMER_C3) = now + TIMER_INTERVAL;
    *(volatile uint32_t *)(SYS_TIMER_CS) = (1 << 3);
}

uint64_t GetCounterFreq()
{
    uint64_t freq;
    asm volatile("mrs %0, cntfrq_el0" : "=r" (freq));
    return freq;
}

void localTimerReset()
{
    asm volatile("msr cntp_tval_el0, %0" :: "r"(TIMER_INTERVAL));
}

void localTimerIrqInit() 
{
    //Reset timer
    localTimerReset();
    
    //Enable local timer
    asm volatile("msr cntp_ctl_el0, %0" :: "r"(1));

    //Tie IRQ to timer
    *(volatile uint32_t *)CORE0_TIMER_INTERRUPT_CTRL |= (1 << 1); //CNT_PNS_IRQ

    //Unamsk irq
    enable_irq();
}