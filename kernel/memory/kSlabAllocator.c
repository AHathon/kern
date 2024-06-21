#include "kernel/memory/kSlabAllocator.h"

kSlab kSlabAlloc(unsigned long size)
{
	kSlab slab;
	slab.pagePtr = AllocPages(size);
	slab.size = size;
	return slab;
}

void kSlabFree(kSlab slab)
{
	FreePages(slab.pagePtr);
}