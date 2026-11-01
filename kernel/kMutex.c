#include "kernel/kMutex.h"
#include "kernel/kMutexAllocator.h"

kMutex_t *kMutex_alloc()
{
    return (kMutex_t*)kSlabAlloc_Allocate(kMutexAllocator_GetInst());
}

void kMutex_Init(kMutex_t *mutex)
{
    mutex->isLocked = 0;
    mutex->ownerThread = 0;
}

void kMutex_Lock(kMutex_t *mutex)
{
    while(1)
    {
        uint64_t daif;
        asm volatile("mrs %0, daif" : "=r"(daif));
        asm volatile("msr daifset, #0x2");
        uint32_t wasLocked = mutex->isLocked;
        mutex->isLocked = 1;
        asm volatile("msr daif, %0" : : "r"(daif));

        if(!wasLocked)
            break;
        asm volatile("wfe");
    }

    mutex->ownerThread = kScheduler_GetCurrentThread();
}

void kMutex_Unlock(kMutex_t *mutex)
{
    if(!mutex->isLocked)
        return;

    if(mutex->ownerThread != kScheduler_GetCurrentThread())
        return;

    mutex->ownerThread = 0;
    __atomic_store_n(&mutex->isLocked, 0, __ATOMIC_RELEASE);
    asm volatile("sev");
}
