#include "kernel/kThread.h"

kThread *kThread_Create(void *funcPtr, size_t stackSize, ThreadType type)
{
    kThread *thread = kMemAlloc(sizeof(kThread));
    thread->id = newId++;
    kprintf("kThread_Create: thread->id: %d\n", thread->id);
    thread->state = STATE_READY;
    thread->threadType = type;
    thread->stackSize = stackSize;
    thread->stackPtr = kMemAlloc(stackSize);
    thread->funcPtr = funcPtr;
}

void kThread_Destroy(kThread *thread)
{
    thread->state = STATE_TERMINATED;
    kMemFree(thread->stackPtr, thread->stackSize);
    thread->stackPtr = 0;
    kMemFree(thread, sizeof(kThread));
}