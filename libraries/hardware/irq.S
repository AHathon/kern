#include "libraries/hardware/mmio.h"

.global enable_irq;
.type enable_irq, %function;
enable_irq:
	msr daifclr, #0x2
	isb
	ret

.global disable_irq;
.type disable_irq, %function;
disable_irq:
	msr daifset, #0x2
	isb
	ret
