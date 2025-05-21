.global GetCurrentEL
GetCurrentEL:
    mrs x0, CurrentEL
    and x0, x0, #12
    lsr x0, x0, #2
    ret