#include "kernel/kThread.h"
#include "kernel/kProcess.h"
#include "kernel/memory/kMemoryManager.h"

kThread *kThread_Create(void *parent, void *entryPtr, size_t stackSize, ThreadType type)
{
    kThread *thread = kMemAlloc(sizeof(kThread));

    thread->id = newId++;
    thread->parent = (uintptr_t)parent;
    thread->state = STATE_READY;
    thread->threadType = type;
    thread->stackSize = stackSize;
    kProcess *p = (kProcess*)parent;
    thread->stackBase = kMemAlloc(stackSize);
    MMU_MapMemPages(p->pageTables, (uintptr_t)(thread->stackBase - KERNEL_VIRT_BASE), (uintptr_t)(thread->stackBase - KERNEL_VIRT_BASE), stackSize, type == THREAD_KERNEL);
    thread->entryPtr = entryPtr;
    LOG("Creating thread [id:%d]\n", thread->id);

    return thread;
}

void kThread_Destroy(kThread *thread)
{
    thread->state = STATE_TERMINATED;
    kMemFree(thread->stackBase, thread->stackSize);
    thread->stackBase = 0;
    kMemFree(thread, sizeof(kThread));
}