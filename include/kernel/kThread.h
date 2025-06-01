#pragma once

#include "libraries/types.h"
#include "kernel/memory/kMemoryManager.h"

#define MAX_THREADS 256

static uint64_t newId = 1;

typedef enum
{
    THREAD_USER,
    THREAD_KERNEL
} ThreadType;

typedef enum
{
    STATE_READY,
    STATE_RUNNING,
    STATE_TERMINATED
} ThreadState;

typedef struct
{
    uint64_t id;
    void *funcPtr;
    void *stackPtr;
    size_t stackSize;
    ThreadType threadType;
    ThreadState state;
} kThread;

kThread *kThread_Create(void *funcPtr, size_t stackSize, ThreadType type);
void kThread_Destroy(kThread *thread);