#include "kernel/kThread.h"
#include "kernel/kProcess.h"
#include "kernel/kThreadAllocator.h"
#include "kernel/memory/kMemoryManager.h"
#include "kernel/memory/kMemoryMap.h"

kThread *kThread_Create(void *parent, void *entryPtr, size_t stackSize, ThreadType type)
{
    size_t kernStackSize = PAGE_SIZE * 2;

    kThread *thread = (kThread*)kSlabAlloc_Allocate(kThreadAllocator_GetInst());

    thread->id = newId++;
    thread->parent = (uintptr_t)parent;
    thread->state = STATE_READY;
    thread->threadType = type;
    thread->stackSize = stackSize;
    kProcess *p = (kProcess*)parent;
    thread->entryPtr = entryPtr;

    thread->stackBase = KERN_VADDR_TO_PADDR(kMemAlloc(stackSize));
    MMU_MapMemPages(p->pageTables, (uintptr_t)(thread->stackBase), (uintptr_t)(thread->stackBase), stackSize, type == THREAD_KERNEL);
    thread->sp = (uintptr_t)thread->stackBase + stackSize - (stackSize / 2);

    thread->contextStack = kMemAlloc(kernStackSize);
    MMU_MapMemPages(p->pageTables, (uintptr_t)KERN_VADDR_TO_PADDR(thread->contextStack), (uintptr_t)(thread->contextStack), PAGE_SIZE * 2, 1);
    thread->kern_sp = (uintptr_t)thread->contextStack + kernStackSize - (kernStackSize / 2);

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