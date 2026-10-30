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
    while(__atomic_exchange_n(&mutex->isLocked, 1, __ATOMIC_ACQUIRE))
        asm volatile("wfe");

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
