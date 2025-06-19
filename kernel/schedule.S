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
    ldp x8, x9, [sp, #16 * 15]
    ldp x10, x11, [sp, #16 * 16]
    msr ELR_EL1, x8
    msr SPSR_EL1, x9
    msr SP_EL0, x10
    msr TPIDR_EL0, x11
    b _end

_handle_new_process:
    mov x9, #0
    mov x11, #0

    msr ELR_EL1, x0
    msr SPSR_EL1, x9
    msr SP_EL0, x1
    msr TPIDR_EL0, x11

    ldr x0, =SCHEDULE_TIMER_INTERVAL
    msr cntp_tval_el0, x0

    eret

_end:
    ret
