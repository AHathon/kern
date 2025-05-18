#pragma once

#include "libraries/types.h"
#include "kernel/memory/kMemoryPage.h"
#include "kernel/kPageAllocator.h"

#define KERN_HEAP_START 0xC0000000

typedef struct {
	int32_t pageIndex;
	size_t size;
} kSlab;

kSlab kSlabAlloc(size_t size);
void kSlabFree(kSlab slab);