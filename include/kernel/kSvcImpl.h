#pragma once

#include "libraries/hardware/debug.h"

long svcImpl_MapMem(void *addr, long size);
void svcImpl_FreeMem(void *addr);
void svcImpl_ExitProcess();
long svcImpl_CreateProcess(const char *name, const void *code, long size);
void svcImpl_Debug(void);

void svc_unknown(uint8_t svc);