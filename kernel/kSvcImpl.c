#include "kernel/kSvcImpl.h"

void svcMapMem(void *addr, long size)
{
    kprintf("svcMapMem()");
}

void svcFreeMem(void *addr)
{
    kprintf("svcFreeMem()");
}

void svcDebug(void)
{
    kprintf("svcDebug()");
}