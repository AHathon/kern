#include "kernel/memory/kMemoryManager.h"

void kMemManager_Init()
{
    PageAllocator_Init();
    kSlabAlloc_Init(&slab, 0, 0);
    kprintf("Initialized kMemoryManager\n");
}

kSlab kMemAlloc(size_t size)
{
    kSlabAlloc_Allocate(&slab, size);
    return slab;
}

void kMemFree(kSlab slab)
{
    kSlabAlloc_Free(slab);
}