#pragma once

#include "libraries/types.h"
#include "kernel/memory/kMemoryPage.h"
#include "kernel/memory/kPageAllocator.h"

typedef struct {
	struct kSlab *next;
} kSlab;

void kSlabAlloc_Init(kSlab *slab, size_t objSize, size_t slabSize);
void kSlabAlloc_Allocate(kSlab *slab, size_t size);
void kSlabAlloc_Free(kSlab slab);