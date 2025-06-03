#include "libraries/hardware/constants.h"

.section ".text.kernel"

.global _start_kernel
_start_kernel:
    //skip setup if virt mem is setup
    cmp x1, #1
    beq setup_virt_stack
    
    mov x8, x0
    //set stack to phys
    ldr x0, =__stack_top
    ldr x11, =KERNEL_VIRT_BASE
    sub x0, x0, x11
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
    mov x0, x8
    bl MMU_SetupVirtKernelSpace
    b MMU_ClearIdentityMap

setup_virt_stack:
    //set stack to virtual
    msr  spsel, #1
    ldr x11, =KERNEL_VIRT_BASE
    mov x0, sp
    add x0, x0, x11
    mov sp, x0

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
