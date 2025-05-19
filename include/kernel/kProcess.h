#pragma once

#include "kernel/kThread.h"
#include "libraries/types.h"
#include "kernel/kCodeSet.h"

#define IS_ACTIVE_PROC 1 << 0

typedef struct{
    uint64_t PID;
    uintptr_t memoryPool; //pointer to L0 page
    uint64_t memoryPoolSize;
    uint32_t flags;
    kThread mainThread;
    kCodeSet code;
} kProcess;

void kProcess_Initialize(kProcess *proc);
void kProcess_Start(kProcess *proc);