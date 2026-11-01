#include "libraries/hardware/cache.h"

#define CACHE_LINE_SIZE 64UL

void Cache_CleanInvalidate(const volatile void *addr, size_t size)
{
    uintptr_t end = (uintptr_t)addr + size;
    for(uintptr_t line = (uintptr_t)addr & ~(CACHE_LINE_SIZE - 1); line < end; line += CACHE_LINE_SIZE)
        asm volatile("dc civac, %0" : : "r"(line));
    asm volatile("dsb sy");
}

void Cache_SyncCode(const void *addr, size_t size)
{
    uintptr_t start = (uintptr_t)addr & ~(CACHE_LINE_SIZE - 1);
    uintptr_t end = (uintptr_t)addr + size;
    for(uintptr_t line = start; line < end; line += CACHE_LINE_SIZE)
        asm volatile("dc cvau, %0" : : "r"(line));
    asm volatile("dsb ish");
    for(uintptr_t line = start; line < end; line += CACHE_LINE_SIZE)
        asm volatile("ic ivau, %0" : : "r"(line));
    asm volatile("dsb ish");
    asm volatile("isb");
}
