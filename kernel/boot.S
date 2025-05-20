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
    cbz w6, set_mmio_base
    str xzr, [x5], #8
    sub w6, w6, #1
    cbnz w6, memset

set_mmio_base:
    mov x9, #0x3F000000
    ldr x10, =MMIO_BASE
    str x9, [x10]
    
setup_uart:
    bl UART1_Init

setup_mmu:
    bl VMM_Init

boot_kernel:
    ldr x1, =__stack_top
    mov sp, x1
    
    mov x1, #0
    mov x2, #0
    mov x3, #0
    bl kMain

loop:
    wfi
    b loop

.section ".kips"
.INCBIN "KIPs/kip_blob"
