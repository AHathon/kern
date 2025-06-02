#include "kernel/kSvcImpl.h"

void svcImpl_MapMem(void *addr, long size)
{
    kprintf("svcMapMem()\n");
}

void svcImpl_FreeMem(void *addr)
{
    kprintf("svcFreeMem()\n");
}

void svcImpl_Debug(void)
{
    kprintf("svcDebug()\n");
}