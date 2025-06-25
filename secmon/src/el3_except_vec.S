#include "libraries/hardware/mmio.h"

.section .text

.equ	SYNC_INVALID_el3t,	0
.equ	IRQ_INVALID_el3t,	1
.equ	FIQ_INVALID_el3t,	2
.equ	ERROR_INVALID_el3t,	3

.equ	SYNC_INVALID_el3h,	4
.equ	IRQ_INVALID_el3h,	5
.equ	FIQ_INVALID_el3h,	6
.equ	ERROR_INVALID_el3h,	7

.equ	SYNC_INVALID_EL0_64,	8
.equ	IRQ_INVALID_EL0_64,	9
.equ	FIQ_INVALID_EL0_64,	10
.equ	ERROR_INVALID_EL0_64,	11

.equ	SYNC_INVALID_EL0_32,	12
.equ	IRQ_INVALID_EL0_32,	13
.equ	FIQ_INVALID_EL0_32,	14
.equ	ERROR_INVALID_EL0_32,	15

.extern smc_table

.macro	ventry	label
.balign	128
	b	\label
.endm

.macro secmon_entry
    sub sp, sp, #(16 * 16)

    // Caller-saved registers, FP
    stp x0, x1, [sp, #16 * 0]
    stp x2, x3, [sp, #16 * 1]
    stp	x4, x5, [sp, #16 * 2]
    stp	x6, x7, [sp, #16 * 3]
    stp	x8, x9, [sp, #16 * 4]
    stp	x10, x11, [sp, #16 * 5]
    stp	x12, x13, [sp, #16 * 6]
    stp	x14, x15, [sp, #16 * 7]
    stp	x16, x17, [sp, #16 * 8]
    stp	x18, x19, [sp, #16 * 9]
    stp	x20, x21, [sp, #16 * 10]
    stp	x22, x23, [sp, #16 * 11]
    stp	x24, x25, [sp, #16 * 12]
    stp	x26, x27, [sp, #16 * 13]
    stp	x28, x29, [sp, #16 * 14]
    stp	x30, xzr, [sp, #16 * 15]
.endm

.macro secmon_exit
    // Restore caller-saved registers
    ldp	x0, x1, [sp, #16 * 0]
    ldp	x2, x3, [sp, #16 * 1]
    ldp	x4, x5, [sp, #16 * 2]
    ldp	x6, x7, [sp, #16 * 3]
    ldp	x8, x9, [sp, #16 * 4]
    ldp	x10, x11, [sp, #16 * 5]
    ldp	x12, x13, [sp, #16 * 6]
    ldp	x14, x15, [sp, #16 * 7]
    ldp	x16, x17, [sp, #16 * 8]
    ldp	x18, x19, [sp, #16 * 9]
    ldp	x20, x21, [sp, #16 * 10]
    ldp	x22, x23, [sp, #16 * 11]
    ldp	x24, x25, [sp, #16 * 12]
    ldp	x26, x27, [sp, #16 * 13]
    ldp	x28, x29, [sp, #16 * 14]
    ldp	x30, xzr, [sp, #16 * 15]

    add	sp, sp, #(16 * 16)
.endm

.extern invalid_exception
.macro handle_interrupt type

	ldr	x0, =\type
	bl	invalid_exception


	eret
.endm

sync_el3h:
    secmon_entry
    mrs x25, esr_el3
	lsr x24, x25, #26 
	cmp x24, #EC_SMC_64
	b.eq smc_64
	b sync_invalid_el3h

smc_64:
	and x26, x25, 0xFFFF
	adr x27, smc_table
	mov x25, #NR_SYSCALLS
	cmp x26, x25
	b.hs sync_invalid_el3h
	ldr x16, [x27, x26, lsl #3]
	bl disable_irq
	blr x16	
	bl enable_irq
	secmon_exit
	eret

.align 11
.global el3_vector_base;
.type el3_vector_base, %function;
el3_vector_base:
	ventry	sync_invalid_el3t			// Synchronous el3t
	ventry	irq_invalid_el3t			// IRQ el3t
	ventry	fiq_invalid_el3t			// FIQ el3t
	ventry	error_invalid_el3t			// Error el3t

	// Current EL with SPx
	ventry	sync_el3h					// Synchronous el3h
	ventry	irq_invalid_el3h					// IRQ el3h
	ventry	fiq_invalid_el3h			// FIQ el3h
	ventry	error_invalid_el3h			// Error el3h

	// Lower EL using AArch64
	ventry	sync_el3h					// Synchronous 64-bit EL0
	ventry	irq_invalid_el0_64					// IRQ 64-bit EL0
	ventry	fiq_invalid_el0_64			// FIQ 64-bit EL0
	ventry	error_invalid_el0_64		// Error 64-bit EL0

	// Lower EL using AArch32
	ventry	sync_invalid_el0_32			// Synchronous 32-bit EL0
	ventry	irq_invalid_el0_32			// IRQ 32-bit EL0
	ventry	fiq_invalid_el0_32			// FIQ 32-bit EL0
	ventry	error_invalid_el0_32		// Error 32-bit EL0

sync_invalid_el3t:
	handle_interrupt  SYNC_INVALID_el3t

irq_invalid_el3t:
	handle_interrupt  IRQ_INVALID_el3t

fiq_invalid_el3t:
	handle_interrupt  FIQ_INVALID_el3t

error_invalid_el3t:
	handle_interrupt  ERROR_INVALID_el3t

sync_invalid_el3h:
	handle_interrupt  SYNC_INVALID_el3h

irq_invalid_el3h:
	handle_interrupt  IRQ_INVALID_el3h

fiq_invalid_el3h:
	handle_interrupt  FIQ_INVALID_el3h

error_invalid_el3h:
	handle_interrupt  ERROR_INVALID_el3h

sync_invalid_el0_64:
	handle_interrupt  SYNC_INVALID_EL0_64

irq_invalid_el0_64:
	handle_interrupt  IRQ_INVALID_EL0_64

fiq_invalid_el0_64:
	handle_interrupt  FIQ_INVALID_EL0_64

error_invalid_el0_64:
	handle_interrupt  ERROR_INVALID_EL0_64

sync_invalid_el0_32:
	handle_interrupt  SYNC_INVALID_EL0_32

irq_invalid_el0_32:
	handle_interrupt  IRQ_INVALID_EL0_32

fiq_invalid_el0_32:
	handle_interrupt  FIQ_INVALID_EL0_32

error_invalid_el0_32:
	handle_interrupt  ERROR_INVALID_EL0_32