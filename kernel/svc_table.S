.globl hndlr_svcMapMem
hndlr_svcMapMem:
    b svcImpl_MapMem

.globl hndlr_svcFreeMem
hndlr_svcFreeMem:
    b svcImpl_FreeMem

.globl hndlr_svcDebug
hndlr_svcDebug:
    b svcImpl_Debug

.globl swi_table
swi_table:
    .dword hndlr_svcMapMem
    .dword hndlr_svcFreeMem
    .dword hndlr_svcDebug