.section ".text.kernel"

.global _start_kernel
_start_kernel:
    mov x8, x0
    ldr x0, =__stack_top
    mov x11, #0xFFFFFF8000000000
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
    bl SetupKernelVMM
    ldr x0, =__stack_top
    mov sp, x0

    mov x11, #0xFFFFFF8000000000
    mov x9, #0x3F000000
    add x9, x9, x11
    ldr x10, =MMIO_BASE
    str x9, [x10]

boot_kernel:
    mov x1, #0
    mov x2, #0
    mov x3, #0
    bl kMain

loop:
    wfi
    b loop

.section ".kips", "a"
.align 12
.global __kips_start
__kips_start:
    .incbin "../KIPs/kip_blob"
