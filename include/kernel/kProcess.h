#pragma once

#include "libraries/types.h"
#include "kCodeSet.h"

#define IS_INACTIVE_PROC 0
#define IS_ACTIVE_PROC 1 << 0

typedef struct {
    uint64_t PID;
    uintptr_t heap;
    uint64_t heapSize;
    uint32_t flags;
    kCodeSet code;
} KProcess;