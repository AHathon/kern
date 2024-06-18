#include "kernel/exceptions.h"
#include "kernel/svc.h"

void SetExceptionVecTbl(uint64_t excepVec) {
    asm volatile ("dsb ish; isb; msr vbar_el1, %0" : "=r" (excepVec));
}

//Privledged SVC
void current_el1_sync() {
	//TODO
}

void current_el1_irq() {
	
}

void current_el1_serr() {
	
}

//User-mode SVC
void lower_el1_sync() {
	//TODO
}

void lower_el1_irq() {
	
}

void lower_el1_serr() {
	
}

void undef_vec_ent() {
	
}