#include "kernel/kScheduler.h"

void kScheduler_Init()
{
    currThread = 0;
    setup_irq_timer();
    kprintf("Initialzed scheduler\n");
}

void context_switch()
{
    while(!(threadList[currThread].flags & FLAG_IS_ACTIVE)) 
        currThread++;

    currThread = currThread >= lastPID ? 0 : currThread + 1;
}