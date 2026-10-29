#pragma once

#include "kernel/kProcess.h"
#include "kernel/kScheduler.h"
#include "kernel/memory/kMemoryManager.h"

#define MAX_PROC 256

void kProcessManager_Init();
int64_t kProcessManager_CreateProcess(char *name, uint8_t *code, size_t codeSize, uint8_t isKernelProc, uint8_t start);
int64_t kProcessManager_StartProcess(int64_t pid);
kProcess *kProcessManager_GetProcess(int64_t pid);
kProcess *kProcessManager_GetCurrentProcess();
void kProcessManager_KillProcess(unsigned ind);
void kProcessManager_PrintDebugProc();