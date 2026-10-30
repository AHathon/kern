#include "kernel/kSvcImpl.h"
#include "kernel/kProcessManager.h"
#include "kernel/kScheduler.h"
#include "kernel/memory/kMemoryMap.h"
#include "kernel/memory/mmu.h"
#include "libraries/services/svc.h"
#include "libraries/services/ipc.h"
#include "kernel/kIpc.h"

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

static long svc_CopyName(uintptr_t pageTable, const char *name, char out[IPC_PORT_NAME_LEN])
{
    size_t n = 0;
    for(; n < IPC_PORT_NAME_LEN - 1; n++)
    {
        if(!svc_IsRangeMapped(pageTable, (uintptr_t)&name[n], 1))
            return SVC_ERR_INVALID_ADDR;
        if(!name[n])
            break;
        out[n] = name[n];
    }
    out[n] = 0;
    return n == 0 ? SVC_ERR_INVALID_ADDR : SVC_OK;
}

long svcImpl_CreateProcess(const char *name, const void *code, long size)
{
    kProcess *proc = kProcessManager_GetCurrentProcess();
    uintptr_t pageTable = KERN_PADDR_TO_VADDR(proc->pageTables);

    if(size <= 0)
        return SVC_ERR_INVALID_SIZE;
    if(!svc_IsRangeMapped(pageTable, (uintptr_t)code, size))
        return SVC_ERR_INVALID_ADDR;

    char procName[IPC_PORT_NAME_LEN];
    long r = svc_CopyName(pageTable, name, procName);
    if(r != SVC_OK)
        return r;

    int64_t pid = kProcessManager_CreateProcess(procName, (uint8_t*)code, size, 0, 0);
    return pid < 0 ? SVC_ERR_PROC_LIMIT : pid;
}

long svcImpl_StartProcess(long pid)
{
    return kProcessManager_StartProcess(pid) == 0 ? SVC_OK : SVC_ERR_INVALID_PROCESS;
}

long svcImpl_MapPhysicalMemory(long pid, void *addr, uintptr_t paddr, long size)
{
    kProcess *caller = kProcessManager_GetCurrentProcess();
    if(caller->mainThread->threadType != THREAD_KERNEL)
        return SVC_ERR_NOT_PERMITTED;

    kProcess *target = kProcessManager_GetProcess(pid);
    if(!target)
        return SVC_ERR_INVALID_PROCESS;

    uintptr_t va = (uintptr_t)addr;
    if(va == 0 || (va & (PAGE_SIZE - 1)) || va >= USERLAND_VIRT_END || (paddr & (PAGE_SIZE - 1)))
        return SVC_ERR_INVALID_ADDR;
    if(size <= 0 || size > USERLAND_VIRT_END - va)
        return SVC_ERR_INVALID_SIZE;

    size_t mapSize = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    if(paddr < DRAM_START + MAX_PAGES * PAGE_SIZE)
        return SVC_ERR_INVALID_ADDR;

    uintptr_t pageTable = KERN_PADDR_TO_VADDR(target->pageTables);
    for(size_t off = 0; off < mapSize; off += PAGE_SIZE)
        if(MMU_IsPageMapped(pageTable, va + off))
            return SVC_ERR_ALREADY_MAPPED;

    uint64_t flags = ATTR_NOEXEC | (paddr >= MMIO_BASE ? ATTR_DEVICE : ATTR_UNCACHED);
    if(target->mainThread->threadType == THREAD_KERNEL)
        flags |= ATTR_KERNEL;
    MMU_MapMemPages(pageTable, paddr, va, mapSize, flags);
    return SVC_OK;
}

long svcImpl_CreatePort(const char *name)
{
    kProcess *proc = kProcessManager_GetCurrentProcess();
    char portName[IPC_PORT_NAME_LEN];
    long r = svc_CopyName(KERN_PADDR_TO_VADDR(proc->pageTables), name, portName);
    return r != SVC_OK ? r : kIpc_CreatePort(portName);
}

long svcImpl_ConnectToPort(const char *name)
{
    kProcess *proc = kProcessManager_GetCurrentProcess();
    char portName[IPC_PORT_NAME_LEN];
    long r = svc_CopyName(KERN_PADDR_TO_VADDR(proc->pageTables), name, portName);
    return r != SVC_OK ? r : kIpc_ConnectToPort(portName);
}

long svcImpl_SendSyncRequest(long session)
{
    return kIpc_SendSyncRequest(session);
}

long svcImpl_ReplyAndReceive(long port, long replyTo)
{
    return kIpc_ReplyAndReceive(port, replyTo);
}

long svcImpl_CloseHandle(long handle)
{
    return kIpc_CloseHandle(handle);
}

long svcImpl_CreateSharedMemory(long size)
{
    return kIpc_CreateSharedMemory(size);
}

long svcImpl_MapSharedMemory(long handle, void *addr)
{
    return kIpc_MapSharedMemory(handle, (uintptr_t)addr);
}

long svcImpl_Yield()
{
    kScheduler_Yield();
}

void svcImpl_Debug(const char *msg)
{
    kProcess *proc = kProcessManager_GetCurrentProcess();
    uintptr_t pageTable = KERN_PADDR_TO_VADDR(proc->pageTables);

    char buf[100];
    size_t n = 0;
    for(; n < sizeof(buf) - 1; n++)
    {
        if(!svc_IsRangeMapped(pageTable, (uintptr_t)&msg[n], 1) || !msg[n])
            break;
        buf[n] = msg[n];
    }
    buf[n] = 0;
    LOG("[%s] %s\n", proc->name, buf);
}

//-----------------

void svc_unknown(uint8_t svc)
{
    LOGT("Unknown svc %d\n", svc);
}