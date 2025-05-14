#pragma once

#include "libraries/types.h"
#include "kernel/exceptionDef.h"
#include "debug.h"

extern char el1_vector_base[1];
extern void (*swi_table[NR_SYSCALLS])(volatile unsigned* regs);

extern void SetExceptionVec_El1(uint64_t addr);

void InvalidException(void* ex);
void data_abort_exception(uint64_t status);
void timer_irq_handle();
void execute_EL0(void *page_tbl, void *sp, void *pc);