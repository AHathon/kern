.globl hndlr_smcTest
hndlr_smcTest:
    b smcImpl_Test

.globl hndlr_smcUnknown
hndlr_smcUnknown:
    mov x0, x26
    b smc_unknown

.globl smc_table
smc_table:
    .dword hndlr_smcTest

    .rept 0xFF - 1
    .dword hndlr_smcUnknown
    .endr