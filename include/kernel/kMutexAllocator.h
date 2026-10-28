#pragma once

#include "kernel/memory/kSlabAllocator.h"

#define MAX_MUTEX 32

slab_allocator_t *kMutexAllocator_GetInst();
void kMutexAllocator_DestroyInst();