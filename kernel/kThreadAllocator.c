#include "kernel/kThreadAllocator.h"
#include "kernel/kThread.h"

slab_allocator_t *kThreadAllocator_GetInst()
{
    static slab_allocator_t instance;
    static char initialized = 0;

    if(!initialized)
    {
        kSlabAlloc_Init(&instance, sizeof(kThread), MAX_THREADS);
        initialized = 1;
        LOGT("kThreadAllocator initialized\n");
    }

    return &instance;
}

void kThreadAllocator_DestroyInst()
{
    kSlabAlloc_Destroy(kThreadAllocator_GetInst());
}