#include "kernel/memory/kSlabAllocator.h"

void kSlabAlloc_Init(slab_allocator_t *slabAlloc, size_t objSize, size_t objCnt)
{
    slabAlloc->memPtr = (void*)kMemAlloc(objSize * objCnt);
    slabAlloc->objCnt = objCnt;
    slabAlloc->objSize = objSize;
    slabAlloc->head = slabAlloc->memPtr;
    
    //init freelist
    uintptr_t p = (uintptr_t)(slabAlloc->head);
    for(int i = 0; i < objCnt; i++)
    {
        slab_node_t *node = (slab_node_t*)(p);
        p += objSize;
        node->next = (i == objCnt - 1) ? (slab_node_t*)0 : (slab_node_t*)p;
	}

    LOG("Slab allocated for obj size (0x%X) with count (0x%X)\n",  objSize, objCnt);
}

slab_node_t *kSlabAlloc_Allocate(slab_allocator_t *slabAlloc)
{
	slab_node_t *node = 0;
	if(slabAlloc->head != 0)
	{
		node = slabAlloc->head;
		slabAlloc->head = slabAlloc->head->next;
	}
	return node;
}

void kSlabAlloc_Free(slab_allocator_t *slabAlloc, slab_node_t *slab)
{
    slab->next = slabAlloc->head;
    slabAlloc->head = slab;
}

void kSlabAlloc_Destroy(slab_allocator_t *slabAlloc)
{
    size_t totalSize = slabAlloc->objSize * slabAlloc->objCnt;
    PageAllocator_FreePages((uintptr_t)slabAlloc->memPtr, (totalSize + PAGE_SIZE - 1) / PAGE_SIZE);
}