.globl svcMapMem
.type svcMapMem, %function;
svcMapMem:
    svc 0
    ret

.globl svcFreeMem
.type svcFreeMem, %function;
svcFreeMem:
    svc 1
    ret

.globl svcExitProcess
.type svcExitProcess, %function;
svcExitProcess:
    svc 2
    ret

.globl svcDebug
.type svcDebug, %function;
svcDebug:
    svc 0xFF
    ret