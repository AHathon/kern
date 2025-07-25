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
void context_switch(void *entry, uintptr_t sp, uintptr_t kern_sp, uintptr_t pageTable, uint8_t isNew);