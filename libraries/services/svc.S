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

.globl svcCreateProcess
.type svcCreateProcess, %function;
svcCreateProcess:
    svc 3
    ret

.globl svcCreatePort
.type svcCreatePort, %function;
svcCreatePort:
    svc 4
    ret

.globl svcConnectToPort
.type svcConnectToPort, %function;
svcConnectToPort:
    svc 5
    ret

.globl svcSendSyncRequest
.type svcSendSyncRequest, %function;
svcSendSyncRequest:
    svc 6
    ret

.globl svcReplyAndReceive
.type svcReplyAndReceive, %function;
svcReplyAndReceive:
    svc 7
    ret

.globl svcCloseHandle
.type svcCloseHandle, %function;
svcCloseHandle:
    svc 8
    ret

.globl svcYield
.type svcYield, %function;
svcYield:
    svc 9
    ret

.globl svcStartProcess
.type svcStartProcess, %function;
svcStartProcess:
    svc 10
    ret

.globl svcMapPhysicalMemory
.type svcMapPhysicalMemory, %function;
svcMapPhysicalMemory:
    svc 11
    ret

.globl svcCreateSharedMemory
.type svcCreateSharedMemory, %function;
svcCreateSharedMemory:
    svc 12
    ret

.globl svcMapSharedMemory
.type svcMapSharedMemory, %function;
svcMapSharedMemory:
    svc 13
    ret

.globl svcDebug
.type svcDebug, %function;
svcDebug:
    svc 0xFF
    ret