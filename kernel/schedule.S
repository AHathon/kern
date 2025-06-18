#include "libraries/hardware/constants.h"

.global context_switch
.type context_switch, %function;
context_switch:
	//Set stack and create kern address to read from stack
	msr SP_EL0, x0
    mov sp, x1

    //Update MMU with next proc pagetable
    msr ttbr0_el1, x5
    tlbi vmalle1is
    dsb sy
    isb

    cmp w3, #1	//isNew
    beq _handle_new_process
    ret

_handle_new_process:
    mov x8, #0x3C0
    mov x9, x4

    msr SPSR_EL1, x8
    msr ELR_EL1, x9
    ret