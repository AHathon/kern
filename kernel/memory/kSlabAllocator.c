#include "kernel/memory/kSlabAllocator.h"

kSlab kSlabAlloc(unsigned long size)
{
	kSlab slab;
	slab.pageIndex = AllocPages(size);
	slab.size = size;
	return slab;
}

void kSlabFree(kSlab slab)
{
	FreePages(slab.pageIndex, slab.size);
}