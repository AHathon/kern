#pragma once

#include "kernel/kProcess.h"
#include "libraries/hardware/debug.h"
#include "kernel/memory/kMemoryManager.h"

#define MAX_PROC 256

static kProcess processTable[MAX_PROC];
static unsigned lastPID = 0;

void kProcessManager_Init();
void kProcessManager_CreateKProc(size_t stackSize, unsigned flags);
void kProcessManager_KillProcess(unsigned ind);
void kProcessManager_PrintDebugProc();