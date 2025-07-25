#include "libraries/hardware/constants.h"

.global context_switch
.type context_switch, %function;
context_switch:
    //Set stack and create kern address to read from stack
    mov sp, x2

    //Update MMU with next proc pagetable
    msr ttbr0_el1, x3
    tlbi vmalle1is
    dsb sy
    isb

    cmp w4, #1    //isNew
    beq _handle_new_process

_handle_existing_process:
    ldp	x8, x9, [sp, #16 * 16]
	ldp	x10, x11, [sp, #16 * 17]
	msr ELR_EL1, x8
	msr SPSR_EL1, x9
	msr SP_EL0, x10
	msr TPIDR_EL0, x11
	
	// Restore caller-saved registers
	ldp	x0, x1, [sp, #16 * 0]
	ldp	x2, x3, [sp, #16 * 1]
	ldp	x4, x5, [sp, #16 * 2]
	ldp	x6, x7, [sp, #16 * 3]
	ldp	x8, x9, [sp, #16 * 4]
	ldp	x10, x11, [sp, #16 * 5]
	ldp	x12, x13, [sp, #16 * 6]
	ldp	x14, x15, [sp, #16 * 7]
	ldp	x16, x17, [sp, #16 * 8]
	ldp	x18, x19, [sp, #16 * 9]
	ldp	x20, x21, [sp, #16 * 10]
	ldp	x22, x23, [sp, #16 * 11]
	ldp	x24, x25, [sp, #16 * 12]
	ldp	x26, x27, [sp, #16 * 13]
	ldp	x28, x29, [sp, #16 * 14]
	ldp	x30, xzr, [sp, #16 * 15]

	add	sp, sp, #(16 * 18)

    eret

_handle_new_process:
    mov x9, #0
    mov x11, #0

    msr ELR_EL1, x0
    msr SPSR_EL1, x9
    msr SP_EL0, x1
    msr TPIDR_EL0, x11

    eret