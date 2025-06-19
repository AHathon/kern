#include "kernel/kSvcImpl.h"
#include "kernel/kProcessManager.h"
#include "kernel/kScheduler.h"

void svcImpl_MapMem(void *addr, long size)
{
    LOGT("svcMapMem()\n");
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