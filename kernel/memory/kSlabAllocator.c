#include "kernel/memory/kSlabAllocator.h"

kSlab kSlabAlloc(size_t size)
{
	kSlab slab;
	slab.pageIndex = PageAllocator_AllocPages(size);
	slab.size = size;
	return slab;
}

void kSlabFree(kSlab slab)
{
	PageAllocator_FreePages(slab.pageIndex, slab.size);
}