#include "libraries/hardware/constants.h"

.section ".text.kernel"

.global _start_kernel
_start_kernel:
    //if arg == 1, skip mmu setup
    cmp x0, #0
    beq set_phys_stack
    tlbi vmalle1is
    dsb sy
    isb
    b boot_kernel

set_phys_stack:
    //set stack to phys
    ldr x0, =__stack_top
    ldr x11, =KERNEL_VIRT_BASE
    sub x0, x0, x11
    msr spsel, #1
    mov sp, x0

set_exceptions:
    ldr x0, =el1_vector_base
    msr VBAR_EL1, x0
    isb

clear_bss:
    ldr x5, =__bss_start
    sub x5, x5, x11
    ldr x6, =__bss_size

memset:
    cbz w6, setup_mmu
    str xzr, [x5], #8
    sub w6, w6, #8
    cbnz w6, memset

setup_mmu:
    bl MMU_SetupVirtKernelSpace

setup_virt_stack:
    //set stack to virtual
    ldr x11, =KERNEL_VIRT_BASE
    msr spsel, #1
    mov x0, sp
    add x0, x0, x11
    mov sp, x0

    b MMU_ClearIdentityMap

boot_kernel:
    mov x1, #0
    mov x2, #0
    mov x3, #0
    ldr x8, =kMain
    blr x8

loop:
    wfi
    b loop

.section ".kips", "a"
.align 12
.global __kips_start
__kips_start:
    .incbin "../KIPs/kip_blob"
