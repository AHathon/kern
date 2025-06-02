#pragma once

#include "kernel/kProcess.h"
#include "kernel/kScheduler.h"
#include "libraries/hardware/debug.h"
#include "kernel/memory/kMemoryManager.h"

#define MAX_PROC 256

static kProcess processTable[MAX_PROC];
static unsigned lastPID = 0;

void kProcessManager_Init();
void kProcessManager_CreateProcess(char *name, uint8_t *code, size_t codeSize, void *entry);
void kProcessManager_KillProcess(unsigned ind);
void kProcessManager_PrintDebugProc();