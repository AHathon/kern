#include "kernel/memory/kSlabAllocator.h"

void kSlabAlloc_Init(kSlab *slab, size_t objSize, size_t slabSize)
{
	//PageAllocator_AllocPages(slabSize);
}

void kSlabAlloc_Allocate(kSlab *slab, size_t size)
{
	//TODO
}

void kSlabAlloc_Free(kSlab slab)
{
	//PageAllocator_FreePages(slab.pageIndex, slab.size);
}