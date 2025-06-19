#include "kernel/exceptions.h"
#include "kernel/kScheduler.h"
#include "libraries/hardware/timer.h"
#include "libraries/hardware/mmio.h"
#include "libraries/hardware/debug.h"

inline void PrintExceptionInfo()
{
	uint64_t far, esr, elr;
	asm volatile("mrs %0, far_el1" : "=r"(far));
	asm volatile("mrs %0, esr_el1" : "=r"(esr));
	asm volatile("mrs %0, elr_el1" : "=r"(elr));
	LOGT("ESR_EL1: %016X\n", esr);
	LOGT("ELR_EL1: %016X\n", elr);
	LOGT("FAR_EL1: %016X\n", far);
}

inline void PrintTrapFrame(trap_frame_t *tf)
{
	for(int i = 0; i < sizeof(tf->X) / sizeof(uint64_t); i++)
		LOG("X%d: %016X\n", i, tf->X[i]);
	LOG("ELR_EL1: %016X\n", tf->elr_el1);
	LOG("SPSR_EL1: %016X\n", tf->spsr_el1);
	LOG("SP_EL0: %016X\n", tf->sp_el0);
	LOG("TPIDR_EL0: %016X\n", tf->tpidr_el0);
}

void invalid_exception(trap_frame_t *tf, uint64_t err_type)
{
	LOGT("-----Invalid Exception-----\n");
	PrintTrapFrame(tf);
	LOGT("Error Type: %s\n", exceptionTypes[err_type]);
	PrintExceptionInfo();
	panic();
}

void data_abort_exception(uint64_t status)
{
	LOGT("-----Data Abort Exception-----\nStatus code: %d\n", status);
	PrintExceptionInfo();
	panic();
}

void timer_irq_handle(void *sp)
{
	uint32_t irq = *(volatile uint32_t *)GICC_IAR;
	switch(irq)
	{
		case LOCAL_TIMER_IRQ_PNS:
		{
			*(volatile uint32_t *)GICC_EOIR = irq;
			kScheduler_schedule();
			localTimerIrqReset();
			break;
		}
	}
}