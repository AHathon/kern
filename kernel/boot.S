.section ".text.kernel"

.global _start_kernel
_start_kernel:
    ldr x0, =__stack_top
    mov sp, x0

clear_bss:
    // zero bss for crt
    ldr x5, =__bss_start
    ldr x6, =__bss_size

memset:
    cbz w6, setup_mmu
    str xzr, [x5], #8
    sub w6, w6, #1
    cbnz w6, memset

setup_mmu:
    mov x9, #0x3F000000
    ldr x10, =MMIO_BASE
    str x9, [x10]

    bl VMM_Init
    bl MMU_Start

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
    .incbin "KIPs/kip_blob"
