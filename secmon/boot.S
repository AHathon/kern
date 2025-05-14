.section ".text.boot"

.global _start
_start:

    ldr x0, =0x80000     // address where kernel is loaded
    msr elr_el3, x0
    eret       

secmon_init:
    mov x0, #0x1      //Enable non-secure
    msr scr_el3, x0
    ret