#pragma once

#include "libraries/hardware/debug.h"

void svcImpl_MapMem(void *addr, long size);
void svcImpl_FreeMem(void *addr);
void svcImpl_ExitProcess();
void svcImpl_Debug(void);