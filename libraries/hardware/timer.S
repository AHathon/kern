.global setup_timer;
.type setup_timer, %function;
setup_timer:
	msr daifclr, #0x2
    mrs x0, cntfrq_el0
    msr cntp_tval_el0, x0
	mov x0, #1
    msr cntp_ctl_el0, x0
    isb
	ret