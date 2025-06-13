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
    kThread *t = kScheduler_GetCurrentThread();
    t->state == STATE_TERMINATED;
    kScheduler_schedule();
}

void svcImpl_Debug(void)
{
    LOGT("svcDebug()\n");
}