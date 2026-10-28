#include "kernel/kMutexAllocator.h"
#include "kernel/kMutex.h"

slab_allocator_t *kMutexAllocator_GetInst()
{
    static slab_allocator_t instance;
    static char initialized = 0;

    if(!initialized)
    {
        kSlabAlloc_Init(&instance, sizeof(kMutex_t), MAX_MUTEX);
        initialized = 1;
        LOGT("kMutexAllocator initialized\n");
    }

    return &instance;
}

void kMutexAllocator_DestroyInst()
{
    kSlabAlloc_Destroy(kMutexAllocator_GetInst());
}