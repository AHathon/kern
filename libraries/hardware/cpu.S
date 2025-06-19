.global GetCurrentEL
.type GetCurrentEL, %function;
GetCurrentEL:
    mrs x0, CurrentEL
    and x0, x0, #12
    lsr x0, x0, #2
    ret

.global GetCoreID
.type GetCoreID, %function;
GetCoreID:
    mrs x0, mpidr_el1
    and x0, x0, #3
    ret