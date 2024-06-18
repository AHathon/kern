#pragma once

#include "kernel/debug.h"
#include "kernel/memory/kSlabAllocator.h"
#include "kernel/memory/kMemoryPage.h"
#include "kernel/mmu.h"

#define MAX_PAGES 256

static KMemPage pages[MAX_PAGES];

void InitMemoryManager();
void *KMemAlloc(size_t size);
void KMemFree(void *ptr);

void KMemDebugPrintUsed();