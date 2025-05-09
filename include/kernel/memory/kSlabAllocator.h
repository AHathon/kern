#pragma once

#include "libraries/types.h"
#include "kernel/memory/kMemoryPage.h"
#include "kernel/kPageAllocator.h"

#define KERN_HEAP_START 0xC0000000

typedef struct {
	int pageIndex;
	size_t size;
} kSlab;

void kSlabAllocInit();
kSlab kSlabAlloc(unsigned long size);
void kSlabFree(kSlab slab);