#include "libraries/hardware/mmio.h"

.global enable_irq;
.type enable_irq, %function;
enable_irq:
	msr daifclr, #0xF
	ret

.global disable_irq;
.type disable_irq, %function;
disable_irq:
	msr daifset, #0xF
	ret

.global setup_irq_timer;
.type setup_irq_timer, %function;
setup_irq_timer:
	ldr x0, =ENABLE_IRQS_1
	mov w2, #2
	str w2, [x0]
	dsb sy
	isb

	msr daifclr, #0xF
    mrs x0, cntfrq_el0
    msr cntp_tval_el0, x0
	mov x0, #1
    msr cntp_ctl_el0, x0
    isb
	ret