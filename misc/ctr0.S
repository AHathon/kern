.section ".text.start"
.extern _Z4mainv
.global _start
_start:
    mov x0, #0
    mov x1, #0
    bl _Z4mainv
    svc 3