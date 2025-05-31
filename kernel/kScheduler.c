#include "kernel/kScheduler.h"
#include "libraries/hardware/gic.h"
#include "libraries/hardware/timer.h"

void kScheduler_Init()
{
    currThread = 0;
    maxThread = 0;
    setup_timer();
    setup_gic(SYSTEM_TIMER_IRQ_1); //Only C1/C3 should be used for OS
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