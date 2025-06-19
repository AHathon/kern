#include "libraries/hardware/mmio.h"

.section .text

.equ	SYNC_INVALID_EL1t,	0
.equ	IRQ_INVALID_EL1t,	1
.equ	FIQ_INVALID_EL1t,	2
.equ	ERROR_INVALID_EL1t,	3

.equ	SYNC_INVALID_EL1h,	4
.equ	IRQ_INVALID_EL1h,	5
.equ	FIQ_INVALID_EL1h,	6
.equ	ERROR_INVALID_EL1h,	7

.equ	SYNC_INVALID_EL0_64,	8
.equ	IRQ_INVALID_EL0_64,	9
.equ	FIQ_INVALID_EL0_64,	10
.equ	ERROR_INVALID_EL0_64,	11

.equ	SYNC_INVALID_EL0_32,	12
.equ	IRQ_INVALID_EL0_32,	13
.equ	FIQ_INVALID_EL0_32,	14
.equ	ERROR_INVALID_EL0_32,	15

.extern swi_table

.macro	ventry	label
.balign	128
	b	\label
.endm

.macro kernel_entry
	sub 	sp, sp, #(16 * 18)

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

	mrs x8, ELR_EL1
	mrs x9, SPSR_EL1
	mrs x10, SP_EL0
	mrs x11, TPIDR_EL0
	stp	x8, x9, [sp, #16 * 16]
	stp	x10, x11, [sp, #16 * 17]
.endm

.macro kernel_exit
	ldp	x8, x9, [sp, #16 * 16]
	ldp	x10, x11, [sp, #16 * 17]
	msr ELR_EL1, x8
	msr SPSR_EL1, x9
	msr SP_EL0, x10
	msr TPIDR_EL0, x11

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

	add	sp, sp, #(16 * 18)
.endm

.extern invalid_exception
.macro handle_interrupt type
	kernel_entry

	//Pass interrupt type and exception struct on sp
	mov x0, sp
	ldr	x1, =\type
	bl	invalid_exception

	kernel_exit

	eret
.endm

sync_el0_64:
	kernel_entry
	mrs x25, esr_el1				// read the syndrome register
	lsr x24, x25, #26             	// exception class
	cmp x24, #EC_SVC_64  			// SVC in 64-bit state
	b.eq svc_64
	cmp x24, #EC_DATA_ABORT_EL0 	//handle data aborts until mmu is solid
	b.eq _data_abort
	b sync_invalid_el0_64			//sync error

sync_el1h:
	kernel_entry
	mrs x25, esr_el1				// read the syndrome register
	lsr x24, x25, #26             	// exception class
	cmp x24, #EC_SVC_64  			// SVC in 64-bit state
	b.eq svc_64
	cmp x24, #EC_DATA_ABORT_EL1 	//handle data aborts until mmu is solid
	b.eq _data_abort
	b sync_invalid_el1h				//sync error

svc_64:
	and x26, x25, 0xFFFF			//Extract svc immediate #
	adr x27, swi_table              //Load syscall table pointer
	mov x25, #NR_SYSCALLS
	cmp x26, x25                    //Check upper syscall limit
	b.hs sync_invalid_el1h
	ldr x16, [x27, x26, lsl #3]     // address in the syscall table
	msr daifclr, #0x2
	blr x16							// call sys_* routine
	msr daifset, #0x2
	kernel_exit
	eret

_data_abort:
	bl data_abort_exception
	kernel_exit
	eret

irq_el1h:
	msr SPSel, #1
    kernel_entry
	mov x0, sp
	bl timer_irq_handle
	kernel_exit
	eret

irq_el0_64:
    kernel_entry

    // Save return context
    mrs	x0, SPSR_EL1
    mrs	x1, ELR_EL1
    stp	x0, x1, [sp, #16 * 16]

	mov x0, sp
    bl timer_irq_handle

	// Restore return context
    ldp	x0, x1, [sp, #16 * 16]
    msr SPSR_EL1, x0
    msr ELR_EL1, x1	

    kernel_exit
    eret

.global el1_vector_base;
.type el1_vector_base, %function;

el1_vector_base:
	ventry	sync_invalid_el1t			// Synchronous EL1t
	ventry	irq_invalid_el1t			// IRQ EL1t
	ventry	fiq_invalid_el1t			// FIQ EL1t
	ventry	error_invalid_el1t			// Error EL1t

	// Current EL with SPx
	ventry	sync_el1h					// Synchronous EL1h
	ventry	irq_el1h					// IRQ EL1h
	ventry	fiq_invalid_el1h			// FIQ EL1h
	ventry	error_invalid_el1h			// Error EL1h

	// Lower EL using AArch64
	ventry	sync_el0_64					// Synchronous 64-bit EL0
	ventry	irq_el0_64					// IRQ 64-bit EL0
	ventry	fiq_invalid_el0_64			// FIQ 64-bit EL0
	ventry	error_invalid_el0_64		// Error 64-bit EL0

	// Lower EL using AArch32
	ventry	sync_invalid_el0_32			// Synchronous 32-bit EL0
	ventry	irq_invalid_el0_32			// IRQ 32-bit EL0
	ventry	fiq_invalid_el0_32			// FIQ 32-bit EL0
	ventry	error_invalid_el0_32		// Error 32-bit EL0

sync_invalid_el1t:
	handle_interrupt  SYNC_INVALID_EL1t

irq_invalid_el1t:
	handle_interrupt  IRQ_INVALID_EL1t

fiq_invalid_el1t:
	handle_interrupt  FIQ_INVALID_EL1t

error_invalid_el1t:
	handle_interrupt  ERROR_INVALID_EL1t

sync_invalid_el1h:
	handle_interrupt  SYNC_INVALID_EL1h

fiq_invalid_el1h:
	handle_interrupt  FIQ_INVALID_EL1h

error_invalid_el1h:
	handle_interrupt  ERROR_INVALID_EL1h

sync_invalid_el0_64:
	handle_interrupt  SYNC_INVALID_EL0_64

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