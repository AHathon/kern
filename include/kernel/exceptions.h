#pragma once

#include "libraries/types.h"
#include "libraries/hardware/exceptionDef.h"
#include "libraries/hardware/debug.h"
#include "libraries/hardware/uart.h"

extern char el1_vector_base[1];
extern void (*swi_table[NR_SYSCALLS])(volatile unsigned* regs);

void InvalidException(void* ex);
void data_abort_exception(uint64_t status);
void timer_irq_handle();
void execute_EL0(void *page_tbl, void *sp, void *pc);