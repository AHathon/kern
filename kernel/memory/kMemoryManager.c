#include "kernel/memory/kMemoryManager.h"

void kMemManager_Init()
{
    PageAllocator_Init();
    kprintf("Initialized kMemoryManager\n");
}

void *kMemAlloc(size_t size)
{
    //
}

void kMemFree(void *addr)
{
    //
}