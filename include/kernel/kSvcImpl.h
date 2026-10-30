#pragma once

#include "libraries/hardware/debug.h"

long svcImpl_MapMem(void *addr, long size);
void svcImpl_FreeMem(void *addr);
void svcImpl_ExitProcess();
long svcImpl_CreateProcess(const char *name, const void *code, long size);
long svcImpl_StartProcess(long pid);
long svcImpl_MapPhysicalMemory(long pid, void *addr, uintptr_t paddr, long size);
long svcImpl_CreatePort(const char *name);
long svcImpl_ConnectToPort(const char *name);
long svcImpl_SendSyncRequest(long session);
long svcImpl_ReplyAndReceive(long port, long replyTo);
long svcImpl_CloseHandle(long handle);
long svcImpl_CreateSharedMemory(long size);
long svcImpl_MapSharedMemory(long handle, void *addr);
long svcImpl_Yield();
void svcImpl_Debug(const char *msg);

void svc_unknown(uint8_t svc);