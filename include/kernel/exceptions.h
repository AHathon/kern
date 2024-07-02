#pragma once

#include "libraries/types.h"
#include "kernel/svcDef.h"
#include "debug.h"

extern char arm64_excep_vec_tbl[1];
extern void (*swi_table[NR_SYSCALLS])(volatile unsigned* regs);

extern void SetExceptionVec_El1(uint64_t addr);
extern void SetExceptionVec_El2(uint64_t addr);
extern void SetExceptionVec_El3(uint64_t addr);

void InvalidException(void* ex);