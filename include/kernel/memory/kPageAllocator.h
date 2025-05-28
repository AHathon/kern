#pragma once

#include "libraries/hardware/debug.h"
#include "kernel/memory/kSlabAllocator.h"
#include "kernel/memory/kMemoryPage.h"
#include "kernel/memory/mmu.h"

#define MAX_PAGES 256

static char pageBitmap[MAX_PAGES / 8] = {};

void PageAllocator_Init();
int64_t PageAllocator_AllocPages(size_t pageCnt);
void PageAllocator_FreePages(uint64_t page, size_t count);

uint8_t PageAllocator_GetPageStatus(uint64_t page);
uint64_t PageAllocator_UsedPagesCount();

void PageAllocator_DebugPrintPagesUsed();