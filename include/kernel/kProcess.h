#pragma once

#include "libraries/types.h"
#include "kCodeSet.h"

#define IS_ACTIVE_PROC 1 << 0

typedef struct {
    uint64_t PID;
    uintptr_t memoryPool; //pointer to L0 page
    uint64_t memoryPoolSize;
    uint32_t flags;
    kCodeSet code;
} KProcess;