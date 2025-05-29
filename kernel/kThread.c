#include "kernel/kThread.h"

kThread kThread_Create(void *funcPtr, size_t stackSize, ThreadType type)
{
    kThread thread;
    thread.id = newId++;
    thread.state = STATE_READY;
    thread.threadType = type;
    thread.stackSize = stackSize;
    thread.stackPtr = kMemAlloc(stackSize);
    thread.funcPtr = funcPtr;
}

void kThread_Destroy(kThread thread)
{
    thread.state = STATE_TERMINATED;
    kMemFree(thread.stackPtr, thread.stackSize);
    thread.stackPtr = 0;
}