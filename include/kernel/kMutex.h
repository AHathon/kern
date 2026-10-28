#pragma once

#include "libraries/types.h"
#include "kernel/kThread.h"
#include "kernel/kScheduler.h"

typedef struct
{
    volatile uint32_t isLocked;
    kThread *ownerThread;
} kMutex_t;

kMutex_t *kMutex_alloc();
void kMutex_Init(kMutex_t *mutex);
void kMutex_Lock(kMutex_t *mutex);
void kMutex_Unlock(kMutex_t *mutex);