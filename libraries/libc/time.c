#include "libraries/libc/time.h"

uint64_t CounterFreq()
{
    uint64_t v;
    asm volatile("mrs %0, cntfrq_el0" : "=r"(v));
    return v;
}