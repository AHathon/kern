#include "kernel/exceptions.h"
#include "kernel/kScheduler.h"
#include "libraries/hardware/timer.h"
#include "libraries/hardware/mmio.h"
#include "libraries/hardware/debug.h"

void InvalidException(void* ex)
{
	LOG("-----Invalid Exception-----\nStack:\n");
	
	InvalidException_t *except = (InvalidException_t*)ex;
	kHexDump((uint8_t*)except->x, sizeof(except->x));
	LOG("ESR_EL1: %016X\n", except->esr_el1);
	LOG("ELR_EL1: %016X\n", except->elr_el1);
	LOG("FAR_EL1: %016X\n", except->far_el1);
	LOG("Error Type: %s\n", exceptionTypes[except->err_type]);
	while(1);
}

void data_abort_exception(uint64_t status)
{
	LOG("----------\nData Abort Exception:\nStatus code: %d\n", status);
	uint64_t far, esr, elr;
	asm volatile("mrs %0, far_el1" : "=r"(far));
	asm volatile("mrs %0, esr_el1" : "=r"(esr));
	asm volatile("mrs %0, elr_el1" : "=r"(elr));
	LOG("FAR: 0x%X\nELR: 0x%X\nESR: 0x%x\n", far, elr, esr);
	while(1);
}

void timer_irq_handle(void *sp)
{
	uint32_t irq = *(volatile uint32_t *)GICC_IAR;
	switch(irq)
	{
		case LOCAL_TIMER_IRQ_PNS:
		{
			kThread *curr = kScheduler_GetCurrentThread();
			curr->sp = (uintptr_t)sp;
			kScheduler_schedule();
			break;
		}
	}
	*(volatile uint32_t *)GICC_EOIR = irq;
}