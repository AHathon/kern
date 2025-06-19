.globl hndlr_svcMapMem
hndlr_svcMapMem:
    b svcImpl_MapMem

.globl hndlr_svcFreeMem
hndlr_svcFreeMem:
    b svcImpl_FreeMem

.globl hndlr_svcExitProcess
hndlr_svcExitProcess:
    b svcImpl_ExitProcess

.globl hndlr_svcDebug
hndlr_svcDebug:
    b svcImpl_Debug

.globl hndlr_svcUnknown
hndlr_svcUnknown:
    mov x0, x26
    b svc_unknown

.globl swi_table
swi_table:
    .dword hndlr_svcMapMem
    .dword hndlr_svcFreeMem
    .dword hndlr_svcExitProcess

    .rept 0xFF - 3
    .dword hndlr_svcUnknown
    .endr
    
    .dword hndlr_svcDebug