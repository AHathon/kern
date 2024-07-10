.section ".text.boot"

.global _start
_start:
    ldr x1, =el2_stack_end
    mov sp, x1

    //we only want core0
    mrs x1, mpidr_el1
    and x1, x1, #3
    cbz x1, set_el1_exceptions
    b loop

//EL1 = kernel (svc)
//EL2 = hypervisor (hvc)
set_el1_exceptions:
    ldr x0, =arm64_excep_vec_tbl
    bl SetExceptionVec_El1

check_EL:
    bl GetCurrentEL
    cmp x0, #1
    b.eq addr_setup
    cmp x0, #2
    b.eq drop_EL2_to_EL1

drop_EL3_to_EL2:
    mov x2, #0x5b1
    msr scr_el3, x2
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
	adr x0, addr_setup
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
    ldr x0, =el1_stack_end
    msr SP_EL1, x0

    //sync
    dsb ish
    isb

	eret

addr_setup:
    // zero bss for crt
    ldr x5, =__bss_start
    ldr x6, =__bss_size

memset:
    cbz w6, detect_hardware
    str xzr, [x5], #8
    sub w6, w6, #1
    cbnz w6, memset

detect_hardware:
    // load address of MMIO_BASE into x10
    ldr x10, =MMIO_BASE

    // detect board type
    mrs x9, midr_el1
    lsr x9, x9, #4              // turn off all bits except 4:15
    and x9, x9, #0xFFF
    // compare and jump to corresponding branch
    cmp x9, #0xD03
    beq raspi_3
    cmp x9, #0xD08
    beq raspi_4
    // no match, halt
    mov x6, #0
    b loop

raspi_3:
    // raspberry pi 3
    mov x9, #0x3F000000
    b set_mmio_base

raspi_4:
    // raspberry pi 4
    mov x9, #0xFE000000

set_mmio_base:
    str x9, [x10]           // write address to MMIO_BASE

startMain:
    ldr x4, =kMain

boot_kernel:
    mov x1, #0
    mov x2, #0
    mov x3, #0
    br x4

loop:
    wfi
    b loop

.section ".kips"
    .byte 0
