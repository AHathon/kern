#include "kernel/kThread.h"
#include "kernel/kProcess.h"
#include "kernel/kThreadAllocator.h"
#include "kernel/memory/kMemoryManager.h"

kThread *kThread_Create(void *parent, void *entryPtr, size_t stackSize, ThreadType type)
{
    kThread *thread = kSlabAlloc_Allocate(kThreadAllocator_GetInst());

    thread->id = newId++;
    thread->parent = (uintptr_t)parent;
    thread->state = STATE_READY;
    thread->threadType = type;
    thread->stackSize = stackSize;
    kProcess *p = (kProcess*)parent;
    thread->entryPtr = entryPtr;

    thread->stackBase = KERN_VADDR_TO_PADDR(kMemAlloc(stackSize));
    MMU_MapMemPages(p->pageTables, (uintptr_t)(thread->stackBase), (uintptr_t)(thread->stackBase), stackSize, type == THREAD_KERNEL);

    thread->contextStack = kMemAlloc(PAGE_SIZE);
    MMU_MapMemPages(p->pageTables, (uintptr_t)(thread->contextStack), (uintptr_t)KERN_PADDR_TO_VADDR(thread->contextStack), PAGE_SIZE, 1);
    thread->contextStack = KERN_PADDR_TO_VADDR(thread->contextStack);

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