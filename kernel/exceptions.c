#include "kernel/exceptions.h"

void ExceptionVector_Init(uint64_t excepVec) {
    asm volatile ("dsb ish; isb; msr vbar_el1, %0" : "=r" (excepVec));
}

void InvalidException(void* ex)
{
	kprintf("InvalidException!\n");
}

void ExceptionHandler(uint64_t num)
{
	//
}
