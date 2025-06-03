#pragma once

#include "libraries/types.h"
#include "libraries/hardware/uart.h"

typedef struct
{
    uint64_t x[31];
    uint64_t esr_el1;
    uint64_t elr_el1;
    uint64_t far_el1;
    uint64_t err_type;
} InvalidException_t;

char *exceptionTypes[] = {
    "SYNC_INVALID_EL1t",
    "IRQ_INVALID_EL1t",
    "FIQ_INVALID_EL1t",
    "ERROR_INVALID_EL1t",
    "SYNC_INVALID_EL1h",
    "IRQ_INVALID_EL1h",
    "FIQ_INVALID_EL1h",
    "ERROR_INVALID_EL1h",
    "SYNC_INVALID_EL0_64",
    "IRQ_INVALID_EL0_64",
    "FIQ_INVALID_EL0_64",
    "ERROR_INVALID_EL0_64",
    "SYNC_INVALID_EL0_32",
    "IRQ_INVALID_EL0_32",
    "FIQ_INVALID_EL0_32",
    "ERROR_INVALID_EL0_32"
};

extern char el1_vector_base[1];
extern void (*swi_table[NR_SYSCALLS])(volatile unsigned* regs);

void InvalidException(void* ex);
void data_abort_exception(uint64_t status);
void timer_irq_handle(void *sp);
void execute_EL0(void *page_tbl, void *sp, void *pc);