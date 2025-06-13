#pragma once

#include "libraries/types.h"

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
    STATE_WAITING,
    STATE_TERMINATED
} ThreadState;

typedef struct
{
    uint64_t id;
    uintptr_t parent;
    void *entryPtr;
    void *stackBase;
    void *contextStack;
    uintptr_t sp;
    uintptr_t kern_sp;
    size_t stackSize;
    ThreadType threadType;
    ThreadState state;
} kThread;

kThread *kThread_Create(void *parent, void *entryPtr, size_t stackSize, ThreadType type);
void kThread_Destroy(kThread *thread);