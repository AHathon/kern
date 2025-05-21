.global MMU_Start
.type MMU_Start, %function;
MMU_Start:
    mrs x1, sctlr_el1
    orr x1, x1, #1
    msr sctlr_el1, x1
    isb
    ret