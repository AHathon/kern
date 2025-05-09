#include "kernel/memory/kMemoryManager.h"

void kMemManager_Init()
{
    kSlabAllocInit();
    kprintf("Initialized kMemoryManager\n");
}

kSlab kMemAlloc(size_t size)
{
    return kSlabAlloc(size);
}

void kMemFree(kSlab slab)
{
    kSlabFree(slab);
}