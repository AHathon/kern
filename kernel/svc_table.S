.globl hndlr_svcMapMem
hndlr_svcMapMem:
    b svcImpl_MapMem

.globl hndlr_svcFreeMem
hndlr_svcFreeMem:
    b svcImpl_FreeMem

.globl hndlr_svcExitProcess
hndlr_svcExitProcess:
    b svcImpl_ExitProcess

.globl hndlr_svcCreateProcess
hndlr_svcCreateProcess:
    b svcImpl_CreateProcess

.globl hndlr_svcCreatePort
hndlr_svcCreatePort:
    b svcImpl_CreatePort

.globl hndlr_svcConnectToPort
hndlr_svcConnectToPort:
    b svcImpl_ConnectToPort

.globl hndlr_svcSendSyncRequest
hndlr_svcSendSyncRequest:
    b svcImpl_SendSyncRequest

.globl hndlr_svcReplyAndReceive
hndlr_svcReplyAndReceive:
    b svcImpl_ReplyAndReceive

.globl hndlr_svcCloseHandle
hndlr_svcCloseHandle:
    b svcImpl_CloseHandle

.globl hndlr_svcYield
hndlr_svcYield:
    b svcImpl_Yield

.globl hndlr_svcStartProcess
hndlr_svcStartProcess:
    b svcImpl_StartProcess

.globl hndlr_svcMapPhysicalMemory
hndlr_svcMapPhysicalMemory:
    b svcImpl_MapPhysicalMemory

.globl hndlr_svcCreateSharedMemory
hndlr_svcCreateSharedMemory:
    b svcImpl_CreateSharedMemory

.globl hndlr_svcMapSharedMemory
hndlr_svcMapSharedMemory:
    b svcImpl_MapSharedMemory

.globl hndlr_svcDebug
hndlr_svcDebug:
    b svcImpl_Debug

.globl hndlr_svcUnknown
hndlr_svcUnknown:
    mov x0, x26
    b svc_unknown

.globl swi_table
swi_table:
    .dword hndlr_svcMapMem
    .dword hndlr_svcFreeMem
    .dword hndlr_svcExitProcess
    .dword hndlr_svcCreateProcess
    .dword hndlr_svcCreatePort
    .dword hndlr_svcConnectToPort
    .dword hndlr_svcSendSyncRequest
    .dword hndlr_svcReplyAndReceive
    .dword hndlr_svcCloseHandle
    .dword hndlr_svcYield
    .dword hndlr_svcStartProcess
    .dword hndlr_svcMapPhysicalMemory
    .dword hndlr_svcCreateSharedMemory
    .dword hndlr_svcMapSharedMemory

    .rept 0xFF - 14
    .dword hndlr_svcUnknown
    .endr
    
    .dword hndlr_svcDebug