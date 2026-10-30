#include "kernel/kThread.h"
#include "kernel/kProcess.h"
#include "kernel/kThreadAllocator.h"
#include "kernel/memory/kMemoryManager.h"
#include "kernel/memory/kMemoryMap.h"
#include "libraries/services/ipc.h"

kThread *kThread_Create(void *parent, void *entryPtr, size_t stackSize, ThreadType type)
{
    size_t kernStackSize = PAGE_SIZE * 2;

    kThread *thread = (kThread*)kSlabAlloc_Allocate(kThreadAllocator_GetInst());

    thread->id = newId++;
    thread->parent = (uintptr_t)parent;
    thread->state = STATE_READY;
    thread->started = 0;
    thread->threadType = type;
    thread->stackSize = stackSize;
    kProcess *p = (kProcess*)parent;
    thread->entryPtr = entryPtr;
    memset((uint8_t*)&thread->fpu, 0, sizeof(thread->fpu));

    uint64_t flags = (type == THREAD_KERNEL) ? ATTR_KERNEL : 0;
    thread->stackBase = KERN_VADDR_TO_PADDR(kMemAlloc(stackSize));
    //TODO: one stack VA per process, same as the IPC buffer
    MMU_MapMemPages(p->pageTables, (uintptr_t)(thread->stackBase), USERLAND_STACK_TOP - stackSize, stackSize, flags | ATTR_NOEXEC);
    thread->sp = USERLAND_STACK_TOP;

    thread->contextStack = kMemAlloc(kernStackSize);
    thread->kern_sp = (uintptr_t)thread->contextStack + kernStackSize;

    //TODO: one IPC VA per process; a second thread would need its own slot
    thread->ipcBuf = kMemCalloc(PAGE_SIZE);
    MMU_MapMemPages(p->pageTables, KERN_VADDR_TO_PADDR((uintptr_t)thread->ipcBuf), IPC_BUFFER_VA, PAGE_SIZE, flags | ATTR_NOEXEC);

    LOG("Creating thread [id:%d]\n", thread->id);

    return thread;
}

void kThread_Destroy(kThread *thread)
{
    thread->state = STATE_TERMINATED;
    kMemFree((void*)KERN_PADDR_TO_VADDR((uintptr_t)thread->stackBase), thread->stackSize);
    thread->stackBase = 0;
    kMemFree(thread->ipcBuf, PAGE_SIZE);
    thread->ipcBuf = 0;
    kSlabAlloc_Free(kThreadAllocator_GetInst(), (slab_node_t*)thread);
}