#pragma once

#include "libraries/types.h"

extern char arm64_excep_vec_tbl[1];

void SetExceptionVecTbl(uint64_t excepVec);

void current_el1_sync(void);
void current_el1_irq(void);
void current_el1_serr(void);
void lower_el1_sync(void);
void lower_el1_irq(void);
void lower_el1_serr(void);
void undef_vec_ent(void);