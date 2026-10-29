#pragma once

#include "kernel/kProcess.h"

#define IPC_MAX_PORTS       32
#define IPC_MAX_SESSIONS    64
#define IPC_MAX_WAITERS     32

long kIpc_CreatePort(const char *name);
long kIpc_ConnectToPort(const char *name);
long kIpc_SendSyncRequest(long session);
long kIpc_ReplyAndReceive(long port, long replyTo);
long kIpc_CloseHandle(long handle);

void kIpc_ProcessExit(kProcess *proc);
