#include "kernel/kScheduler.h"

void kScheduler_init()
{
    currProc = 0;
    setup_irq_timer();
    kprintf("Initialzed scheduler\n");
}

void context_switch()
{
    while(!(processTable[currProc].flags & IS_ACTIVE_PROC)) 
        currProc++;

    currProc = currProc >= lastPID ? 0 : currProc + 1;
}