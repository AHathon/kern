#include "kernel/kScheduler.h"
#include "libraries/hardware/gic.h"
#include "libraries/hardware/timer.h"

void kScheduler_Init()
{
    currThread = 0;
    maxThread = 0;
    BCMTimerReset();
    GicEnable();
    //Only C1/C3 should be used for OS
    GicSetup(SYSTEM_TIMER_IRQ_1);
    GicSetup(SYSTEM_TIMER_IRQ_3);
    //unmask irq
    enable_irq();
    kprintf("Initialzed scheduler\n");
    *(volatile uint32_t *)(GICD_ISPENDR(SYSTEM_TIMER_IRQ_1 / 32)) = (1 << (SYSTEM_TIMER_IRQ_1 % 32));

}

void kScheduler_Start()
{
    //
}

void kScheduler_AddThread(kThread thread)
{
    if(maxThread >= MAX_THREADS)
        return;
    threadList[maxThread++] = thread;
}

void context_switch()
{
    while(!(threadList[currThread].state & STATE_TERMINATED)) 
        currThread++;

    currThread = currThread >= lastPID ? 0 : currThread + 1;
}