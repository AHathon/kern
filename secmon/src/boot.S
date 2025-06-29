#include "libraries/hardware/constants.h"

#define CPUECTLR_EL1    S3_1_C15_C2_1
#define L2CTLR_EL1        S3_1_C11_C0_2

.section ".text.boot"

.globl _start
_start:
    ldr x0, =__stack_top
    mov sp, x0

	b _setup_el3

.ltorg

.org 0xd8
.globl spin_cpu0
spin_cpu0:
    .quad 0
.org 0xe0
.globl spin_cpu1
spin_cpu1:
    .quad 0
.org 0xe8
.globl spin_cpu2
spin_cpu2:
    .quad 0
.org 0xf0
.globl spin_cpu3
spin_cpu3:
    # Shared with next two symbols/.word
    # FW clears the next 8 bytes after reading the initial value, leaving
    # the location suitable for use as spin_cpu3
.org 0xf0
.globl stub_magic
stub_magic:
    .word 0x5afe570b
.org 0xf4
.globl stub_version
stub_version:
    .word 0
.org 0xf8
.globl dtb_ptr32
dtb_ptr32:
    .word 0x0
.org 0xfc
.globl kernel_entry32
kernel_entry32:
    .word 0x0

_setup_el3:
    //Setup security state 
    mov x0, xzr
    orr x0, x0, #(1 << 10)    //Aarch64
    orr x0, x0, #(1 << 5)    //Res1
    orr x0, x0, #(1 << 4)    //Res0
    orr x0, x0, #(1 << 0)    //Non-secure (NSE = 0; NS = 1)
    msr SCR_EL3, x0
	isb

    //Set EL3 exception vec
    ldr x0, =el3_vector_base
    msr VBAR_EL3, x0
	isb

    //Setup timer frequency used for irq 
    ldr x0, =OSC_FREQ
    msr cntfrq_el0, x0

    //Set L2 read/write cache latency to 3
    mrs x0, L2CTLR_EL1
    orr x0, x0, #(1 << 5)
    orr x0, x0, #(1 << 1)
    msr L2CTLR_EL1, x0

    //Trap register
    msr CPTR_EL3, xzr

    //Set up ACTLR
    mov x0, xzr
    orr x0, x0, #(1 << 6)    //Res0
    orr x0, x0, #(1 << 5)    //Error rec regs enable
    orr x0, x0, #(1 << 4)    //Res0
    orr x0, x0, #(1 << 1)    //Ext ctrl regs enable
    orr x0, x0, #(1 << 0)    //Aux ctrl regs enable
    msr ACTLR_EL3, x0

    //Set SMPEN
    mov x0, xzr
    orr x0, x0, #(1 << 6)
    msr CPUECTLR_EL1, x0

    bl secMain
    dsb sy
    isb

    /*
     * Set up SCTLR_EL2
     * All set bits below are res1. LE, no WXN/I/SA/C/A/M
     */
    mov x0, xzr
    orr x0, x0, #(1 << 29)
    orr x0, x0, #(1 << 28)
    orr x0, x0, #(1 << 23)
    orr x0, x0, #(1 << 22)
    orr x0, x0, #(1 << 18)
    orr x0, x0, #(1 << 16)
    orr x0, x0, #(1 << 11)
    orr x0, x0, #(1 << 5)
    orr x0, x0, #(1 << 4)
    msr SCTLR_EL2, x0
	isb

    //Drop to EL2
    mov x0, #9				//Reserved value?
    orr x0, x0, #(1 << 9)
    orr x0, x0, #(1 << 8)
    orr x0, x0, #(1 << 7)
    orr x0, x0, #(1 << 6)
    msr SPSR_EL3, x0
    adr x0, in_el2
    msr ELR_EL3, x0
    eret
in_el2:

    mrs x6, MPIDR_EL1
    and x6, x6, #0x3
    cbz x6, primary_cpu

    adr x5, spin_cpu0
secondary_spin:
    wfe
    ldr x4, [x5, x6, lsl #3]
    cbz x4, secondary_spin
    mov x0, #0
    b boot_kernel

primary_cpu:
    ldr w4, kernel_entry32
    ldr w0, dtb_ptr32

boot_kernel:
    mov x1, #0
    mov x2, #0
    mov x3, #0
    br x4