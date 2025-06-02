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
void kScheduler_schedule(state_ctxt_t ctxt);
void context_switch(state_ctxt_t curr, state_ctxt_t next);