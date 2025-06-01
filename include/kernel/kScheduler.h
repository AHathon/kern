#pragma once

#include "kernel/kProcessManager.h"
#include "libraries/hardware/irq.h"
#include "kernel/kThread.h"

typedef struct {
    kThread *queue[MAX_THREADS];
    int front;
    int back;
    int size;
} run_queue_t;

void kScheduler_Init();
void kScheduler_AddThread(kThread *thread);
void kScheduler_schedule();
void context_switch();