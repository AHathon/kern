#pragma once

#include "libraries/hardware/constants.h"
#include "libraries/types.h"

extern char el3_vector_base[1];
extern void (*smc_table[NR_SYSCALLS])(volatile unsigned* regs);

void invalid_exception(uint64_t err_type);