#include "kernel/kSvcImpl.h"
#include "kernel/kProcessManager.h"
#include "kernel/kScheduler.h"
#include "kernel/memory/kMemoryMap.h"
#include "kernel/memory/mmu.h"
#include "libraries/services/svc.h"

long svcImpl_MapMem(void *addr, long size)
{
    LOGT("svcMapMem() @ 0x%X [0x%X]\n", addr, size);

    uintptr_t va = (uintptr_t)addr;
    if(va == 0 || (va & (PAGE_SIZE - 1)) || va >= USERLAND_VIRT_END)
        return SVC_ERR_INVALID_ADDR;

    if(size <= 0 || size > USERLAND_VIRT_END - va)
        return SVC_ERR_INVALID_SIZE;

    size_t pageCnt = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    if(pageCnt * PAGE_SIZE > USERLAND_VIRT_END - va)
        return SVC_ERR_INVALID_SIZE;

    kProcess *proc = kProcessManager_GetCurrentProcess();
    uintptr_t pageTable = KERN_PADDR_TO_VADDR(proc->pageTables);

    for(size_t i = 0; i < pageCnt; i++)
        if(MMU_IsPageMapped(pageTable, va + i * PAGE_SIZE))
            return SVC_ERR_ALREADY_MAPPED;

    void *mem = kMemCalloc(pageCnt * PAGE_SIZE);
    if(!mem)
        return SVC_ERR_OUT_OF_MEMORY;

    MMU_MapMemPages(pageTable, KERN_VADDR_TO_PADDR((uintptr_t)mem), va, pageCnt * PAGE_SIZE, ATTR_NOEXEC);
    return SVC_OK;
}

void svcImpl_FreeMem(void *addr)
{
    LOGT("svcFreeMem()\n");
}

void svcImpl_ExitProcess()
{
    LOGT("svcExitProcess()\n");
    kProcess *proc = kProcessManager_GetCurrentProcess();
    kProcessManager_KillProcess(proc->PID);
    
    kScheduler_schedule();
}

void svcImpl_Debug(void)
{
    LOGT("svcDebug()\n");
}

//-----------------

void svc_unknown(uint8_t svc)
{
    LOGT("Unknown svc %d\n", svc);
}