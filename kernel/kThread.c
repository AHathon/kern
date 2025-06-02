#include "kernel/kThread.h"
#include "kernel/kProcess.h"
#include "kernel/memory/kMemoryManager.h"

kThread *kThread_Create(void *parent, void *funcPtr, size_t stackSize, ThreadType type)
{
    kThread *thread = kMemAlloc(sizeof(kThread));

    thread->id = newId++;
    thread->parent = (uintptr_t)parent;
    thread->state = STATE_READY;
    thread->threadType = type;
    thread->stackSize = stackSize;
    thread->stackPtr = kMemAlloc(stackSize);
    thread->funcPtr = funcPtr;

    return thread;
}

void kThread_Destroy(kThread *thread)
{
    thread->state = STATE_TERMINATED;
    kMemFree(thread->stackPtr, thread->stackSize);
    thread->stackPtr = 0;
    kMemFree(thread, sizeof(kThread));
}