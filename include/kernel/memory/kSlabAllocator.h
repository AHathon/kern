#pragma once

#include "libraries/types.h"
#include "kernel/memory/kMemoryPage.h"
#include "kernel/memory/kPageAllocator.h"

static int64_t slabStart = -1;

void kSlabAlloc_Init(int64_t *slab, size_t objSize, size_t slabSize);
int64_t kSlabAlloc_Allocate(int64_t *slab, size_t size);
void kSlabAlloc_Free(int64_t *slab, int64_t ptr);
void kSlabAlloc_Destroy(int64_t slab);