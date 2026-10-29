#pragma once

#include "kernel/kThread.h"

typedef struct 
{
    kThread *queue[MAX_THREADS];
    int front;
    int back;
    int size;
} run_queue_t;

static uint8_t schedulerInit = 0;

void kScheduler_Init();
void kScheduler_AddThread(kThread *thread);
void kScheduler_schedule();
kThread *kScheduler_GetCurrentThread();
void kScheduler_SaveTrapFrame(uintptr_t sp);
void kScheduler_SetReturn(kThread *thread, uint64_t value);
void kScheduler_Block() __attribute__((noreturn));
void kScheduler_Wake(kThread *thread, uint64_t retval);
void kScheduler_Yield() __attribute__((noreturn));
void context_switch(void *entry, uintptr_t sp, uintptr_t kern_sp, uintptr_t pageTable, uint8_t isNew, uint8_t isKernel) __attribute__((noreturn));