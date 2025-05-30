#include "kernel/kScheduler.h"

void kScheduler_Init()
{
    currThread = 0;
    maxThread = 0;
    setup_irq_timer();
    kprintf("Initialzed scheduler\n");
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