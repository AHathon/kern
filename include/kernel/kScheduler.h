#pragma once

#include "kernel/kThread.h"

typedef struct 
{
    kThread *queue[MAX_THREADS];
    int front;
    int back;
    int size;
} run_queue_t;

void kScheduler_Init();
void kScheduler_AddThread(kThread *thread);
void kScheduler_schedule();
kThread *GetCurrentThread();
void context_switch(uintptr_t sp, uint8_t isUser);