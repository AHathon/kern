#pragma once

#include "kernel/debug.h"
#include "kernel/memory/kSlabAllocator.h"
#include "kernel/memory/kMemoryPage.h"
#include "kernel/mmu.h"

#define MAX_PAGES 256

static KMemPage pages[MAX_PAGES] = {};

void PageAllocator_Init();
int AllocPages(size_t size);
void FreePages(int index, size_t size);

uintptr_t GetPageAddr(int index);

void DebugPrintPagesUsed();