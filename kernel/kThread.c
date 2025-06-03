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
    thread->stackBase = kMemAlloc(stackSize);
    thread->funcPtr = funcPtr;
    kprintf("Creating thread [id:%d]\n", thread->id);

    return thread;
}

void kThread_Destroy(kThread *thread)
{
    thread->state = STATE_TERMINATED;
    kMemFree(thread->stackBase, thread->stackSize);
    thread->stackBase = 0;
    kMemFree(thread, sizeof(kThread));
}