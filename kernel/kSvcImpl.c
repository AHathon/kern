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

static uint8_t svc_IsRangeMapped(uintptr_t pageTable, uintptr_t va, size_t size)
{
    if(size == 0 || va >= USERLAND_VIRT_END || size > USERLAND_VIRT_END - va)
        return 0;

    for(uintptr_t page = va & ~(PAGE_SIZE - 1); page < va + size; page += PAGE_SIZE)
        if(!MMU_IsPageMapped(pageTable, page))
            return 0;
    return 1;
}

long svcImpl_CreateProcess(const char *name, const void *code, long size)
{
    kProcess *proc = kProcessManager_GetCurrentProcess();
    uintptr_t pageTable = KERN_PADDR_TO_VADDR(proc->pageTables);

    if(size <= 0)
        return SVC_ERR_INVALID_SIZE;
    if(!svc_IsRangeMapped(pageTable, (uintptr_t)code, size))
        return SVC_ERR_INVALID_ADDR;

    //Process names are at most 7 chars + NUL
    char procName[8];
    size_t n = 0;
    for(; n < sizeof(procName) - 1; n++)
    {
        if(!svc_IsRangeMapped(pageTable, (uintptr_t)&name[n], 1))
            return SVC_ERR_INVALID_ADDR;
        if(!name[n])
            break;
        procName[n] = name[n];
    }
    procName[n] = 0;
    if(n == 0)
        return SVC_ERR_INVALID_ADDR;

    int64_t pid = kProcessManager_CreateProcess(procName, (uint8_t*)code, size, 0);
    return pid < 0 ? SVC_ERR_PROC_LIMIT : pid;
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