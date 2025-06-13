#include "kernel/kSvcImpl.h"

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
}

void svcImpl_Debug(void)
{
    LOGT("svcDebug()\n");
}