#include "kernel/kScheduler.h"

void kScheduler_init()
{
    currProc = 0;
    unsigned long freq;
    asm volatile("msr daifclr, #0b0010");
    asm volatile("mrs %0, cntfrq_el0" : "=r"(freq));
    asm volatile("msr cntp_tval_el0, %0" :: "r"(freq));
    asm volatile("msr cntp_ctl_el0, %0" :: "r"(1));
    asm volatile("isb");
}

void kScheduler_add_proc()
{
    //
}

void context_switch()
{
    while(!(processTable[currProc].flags & IS_ACTIVE_PROC)) 
        currProc++;

    currProc = currProc >= lastPID ? 0 : currProc + 1;
}