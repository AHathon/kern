#pragma once

#include "kernel/memory/kSlabAllocator.h"

static kSlab slab;

void kMemManager_Init();
kSlab kMemAlloc(size_t size);
void kMemFree(kSlab slab);