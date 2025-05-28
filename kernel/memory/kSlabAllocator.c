#include "kernel/memory/kSlabAllocator.h"

size_t pageCnt = 0;

void kSlabAlloc_Init(int64_t *slab, size_t objSize, size_t slabSize)
{
	size_t totalSize = objSize * slabSize;
	pageCnt = (totalSize + PAGE_SIZE - 1) / PAGE_SIZE;
	*slab = PageAllocator_AllocPages(pageCnt);
}

int64_t kSlabAlloc_Allocate(int64_t *slab, size_t size)
{
	//TODO: alloc from slab and return offset
	return 0;
}

void kSlabAlloc_Free(int64_t *slab, int64_t ptr)
{
	//TODO: free from slab given ptr
}

void kSlabAlloc_Destroy(int64_t slab)
{
	PageAllocator_FreePages(slab, pageCnt);
}