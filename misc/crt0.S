.section ".text.start"
.extern _Z4mainv
.global _start
_start:
    bl _Z5_initv
    mov x0, #0
    mov x1, #0
    bl _Z4mainv
    mov x19, x0
    bl _Z5_finiv
    mov x0, x19
    svc 2

.weak _Z5_initv
.weak _Z5_finiv
_Z5_initv:
_Z5_finiv:
    ret
