#pragma once

#include "libraries/types.h"

#define SVC_OK                  0
#define SVC_ERR_INVALID_ADDR    -1
#define SVC_ERR_INVALID_SIZE    -2
#define SVC_ERR_ALREADY_MAPPED  -3
#define SVC_ERR_OUT_OF_MEMORY   -4
#define SVC_ERR_PROC_LIMIT      -5
#define SVC_ERR_INVALID_HANDLE  -6
#define SVC_ERR_SESSION_CLOSED  -7
#define SVC_ERR_LIMIT           -8
#define SVC_ERR_BUSY            -9
#define SVC_ERR_NOT_PERMITTED   -10
#define SVC_ERR_INVALID_PROCESS -11

#ifdef __cplusplus
extern "C" {
#endif

long svcMapMem(void *addr, long size);
inline void svcFreeMem(void *addr);
inline void svcExitProcess();
long svcCreateProcess(const char *name, const void *code, long size); //returns PID; not scheduled until svcStartProcess
long svcStartProcess(long pid);
long svcMapPhysicalMemory(long pid, void *addr, uintptr_t paddr, long size);

long svcCreatePort(const char *name);
long svcConnectToPort(const char *name);
long svcSendSyncRequest(long session);
long svcReplyAndReceive(long port, long replyTo);
long svcCloseHandle(long handle);
long svcYield();
void svcDebug(const char *msg);

#ifdef __cplusplus
}
#endif