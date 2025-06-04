#include "kernel/kSvcImpl.h"

void svcImpl_MapMem(void *addr, long size)
{
    LOG("svcMapMem()\n");
}

void svcImpl_FreeMem(void *addr)
{
    LOG("svcFreeMem()\n");
}

void svcImpl_ExitProcess()
{
    LOG("svcExitProcess()\n");
}

void svcImpl_Debug(void)
{
    LOG("svcDebug()\n");
}