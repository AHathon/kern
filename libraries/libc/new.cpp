#include "libraries/libc/stdlib.h"
#include "libraries/services/svc.h"

void *operator new(size_t size)
{
    void *ptr = malloc(size);
    if(!ptr)
    {
        svcDebug("out of memory");
        svcExitProcess();
    }
    return ptr;
}

void operator delete(void *ptr) noexcept
{
    free(ptr);
}

void operator delete(void *ptr, size_t) noexcept
{
    free(ptr);
}
