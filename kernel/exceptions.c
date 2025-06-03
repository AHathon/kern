#include "kernel/exceptions.h"
#include "kernel/kScheduler.h"
#include "libraries/hardware/timer.h"
#include "libraries/hardware/mmio.h"
#include "libraries/hardware/debug.h"

void InvalidException(void* ex)
{
	kprintf("-----Invalid Exception-----\nStack:\n");
	kHexDump(ex, 0x50);
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

void timer_irq_handle(void *sp)
{
	uint32_t irq = *(volatile uint32_t *)GICC_IAR;
	switch(irq)
	{
		case LOCAL_TIMER_IRQ_PNS:
		{
			kThread *curr = GetCurrentThread();
			curr->sp = (uintptr_t)sp;
			localTimerReset();
			kScheduler_schedule();
			break;
		}
	}
	*(volatile uint32_t *)GICC_EOIR = irq;
}