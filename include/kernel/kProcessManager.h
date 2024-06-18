#pragma once

#include "kernel/kProcess.h"
#include "kernel/debug.h"

#define MAX_PROC 256

static KProcess processTable[MAX_PROC];
static unsigned lastPID = 0;

void InitProcessTable();
void CreateKProc(uint64_t memory, unsigned flags);
void KillProcess(unsigned ind);
void startKIPs();