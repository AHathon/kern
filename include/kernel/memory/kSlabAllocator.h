#pragma once

#include "libraries/types.h"
#include "kernel/memory/kMemoryPage.h"
#include "kernel/kPageAllocator.h"

#define KERN_HEAP_START 0xC0000000

typedef struct {
	void *pagePtr;
	size_t size;
} kSlab;

kSlab kSlabAlloc(unsigned long size);
void kSlabFree(kSlab slab);