#include "kernel/exceptions.h"

void InvalidException(void* ex)
{
	//kprintf("InvalidException!\n");
}

void data_abort_exception(uint64_t status)
{
	kprintf("----------\nData Abort Exception:\nStatus code: %d\n", status);
	uint64_t far, esr;
	asm volatile("mrs %0, far_el1" : "=r"(far));
	asm volatile("mrs %0, esr_el1" : "=r"(esr));
	kprintf("Fault Address: 0x%x, ESR: 0x%x\n", far, esr);
	while(1);
}

void timer_irq_handle()
{
	kprintf("timer_irq_handle()\n");
}