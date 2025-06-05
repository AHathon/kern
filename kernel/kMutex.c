#include "kernel/kMutex.h"

void kMutex_Init(kMutex_t *mutex)
{
    mutex->isLocked = 0;
    mutex->ownerThread = 0;
}

void kMutex_Lock(kMutex_t *mutex)
{
    if(mutex->isLocked)
    {
        kThread *currThread = kScheduler_GetCurrentThread();
        currThread->state = STATE_WAITING;
    }
    else
    {
        mutex->isLocked = 1;
        mutex->ownerThread = kScheduler_GetCurrentThread();
    }
}

void kMutex_Unlock(kMutex_t *mutex)
{
    if(mutex->isLocked)
    {
        kThread *currThread = kScheduler_GetCurrentThread();
        if(mutex->ownerThread->id == currThread->id)
        {
            currThread->state = STATE_RUNNING;
            mutex->isLocked = 0;
            mutex->ownerThread = 0;
        }
    }
}