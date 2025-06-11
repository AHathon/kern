#pragma once

#include "kernel/memory/kSlabAllocator.h"

slab_allocator_t *kThreadAllocator_GetInst();
void kThreadAllocator_DestroyInst();