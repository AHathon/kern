#pragma once

#include "kernel/debug.h"
#include "kernel/memory/kSlabAllocator.h"
#include "kernel/memory/kMemoryPage.h"
#include "kernel/mmu.h"

#define MAX_PAGES 256

static KMemPage pages[MAX_PAGES];

void PageAllocator_Init();
void *AllocPages(size_t size);
void FreePages(void *ptr);

void DebugPrintPagesUsed();