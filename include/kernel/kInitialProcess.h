#pragma once

#include "kernel/kProcessManager.h"
#include "kernel/kScheduler.h"
#include "kernel/kThread.h"
#include "libraries/types.h"
#include "libraries/hardware/debug.h"

extern const char *__kips_start;
extern const char *__kips_end;

typedef struct
{
    char magic[8];
    uint64_t totalSize;
    uint64_t codeSize;
    uint32_t headerSize;
} KipHeader;


void kInitialProcess_Setup();