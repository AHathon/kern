#include "kernel/kSvcImpl.h"

void svcMapMem(void *addr, long size)
{
    kprintf("svcMapMem()\n");
}

void svcFreeMem(void *addr)
{
    kprintf("svcFreeMem()\n");
}

void svcDebug(void)
{
    kprintf("svcDebug()\n");
}