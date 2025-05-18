.section ".text.boot"

.global _start
_start:
    //we only want core0
    mrs x1, mpidr_el1
    and x1, x1, #3
    cbz x1, set_exception_vectors
    b loop

//EL1 = kernel (svc)
//EL2 = hypervisor (hvc)
//EL3 = secure monitor (smd)
set_exception_vectors:
    ldr x0, =el1_vector_base
    msr VBAR_EL1, x0
    isb

check_EL:
    bl GetCurrentEL
    cmp x0, #1
    b.eq EL1_start
    cmp x0, #2
    b.eq drop_EL2_to_EL1

drop_EL3_to_EL2:
    mov x2, #0x3c9
    msr spsr_el3, x2

    //Set callback after switch to EL2
    adr x2, drop_EL2_to_EL1
    msr elr_el3, x2

    //sync
    dsb ish
    isb

    eret

drop_EL2_to_EL1:
    //Set callback after switch to EL1 
    mov x0, #0x3c5
	msr spsr_el2, x0
	adr x0, EL1_start
	msr elr_el2, x0

    //Enable aarch64
    mov x0, xzr
	orr x0, x0, #(1 << 31)
	orr x0, x0, #(1 << 1)
	msr hcr_el2, x0

    //Enable CNTP for EL1
    mrs x0, cnthctl_el2
    orr x0, x0, #3
    msr cnthctl_el2, x0
    msr cntvoff_el2, xzr
    
    //Disable coprocessor traps
    mov x0, #0x33FF
    msr cptr_el2, x0
    msr hstr_el2, xzr
    mov x0, #(3 << 20)
    msr cpacr_el1, x0

    //Setup SCTLR access
    mov x2, xzr
    orr x2, x2, #(1 << 11)  //FEAT_ExS: exception exit is context syncing
    orr x2, x2, #(1 << 20)  //FEAT_CSV2: disable SCXTNUM_EL0; use HCR_EL2
    orr x2, x2, #(1 << 22)  //FEAT_ExS: Taking except to EL1 is context syncing
    orr x2, x2, #(1 << 23)  //FEAT_PAN: PSTATE left unchanged on taking EL1 except.
    orr x2, x2, #(1 << 28)  //FEAT_LSMAOC: All memory accesses by A32 and T32 Load Multiple and Store Multiple at EL0 that are marked at stage 1 as ... are not trapped.
    orr x2, x2, #(1 << 29)  //FEAT_LSMAOC: The ordering and interrupt behavior of A32 and T32 Load Multiple and Store Multiple at EL0 is as defined for Armv8.0.
    msr sctlr_el1, x2

    //Set EL1 stack pointer
    ldr x0, =__stack_top
    msr SP_EL1, x0

    //sync
    dsb ish
    isb

	eret

EL1_start:
    nop

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

setup_mmu:
    bl MMU_Init

setup_uart:
    bl UART1_Init

boot_kernel:
    mov x1, #0
    mov x2, #0
    mov x3, #0
    bl kMain

loop:
    wfi
    b loop

.section ".kips"
    .byte 0
