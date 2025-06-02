.globl svcMapMem
.type svcMapMem, %function;
svcMapMem:
    svc 0

.globl svcFreeMem
.type svcFreeMem, %function;
svcFreeMem:
    svc 1

.globl svcDebug
.type svcDebug, %function;
svcDebug:
    svc 2