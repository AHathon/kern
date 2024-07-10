.extern svcMapMem
.globl hndlr_svcMapMem
hndlr_svcMapMem:
    b svcMapMem

.extern svcFreeMem
.globl hndlr_svcFreeMem
hndlr_svcFreeMem:
    b svcFreeMem

.extern svcDebug
.globl hndlr_svcDebug
hndlr_svcDebug:
    b svcDebug

.globl swi_table
swi_table:
    .dword hndlr_svcMapMem
    .dword hndlr_svcFreeMem
    .dword hndlr_svcDebug