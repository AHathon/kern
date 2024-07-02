#pragma once

#include "libraries/types.h"
#include "kernel/svcDef.h"
#include "debug.h"

extern char arm64_excep_vec_tbl[1];
extern void (*swi_table[NR_SYSCALLS])(volatile unsigned* regs);

void ExceptionVector_Init(uint64_t excepVec);
void InvalidException(void* ex);
void ExceptionHandler(uint64_t num);