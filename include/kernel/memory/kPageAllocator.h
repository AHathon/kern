#pragma once

#include "kernel/debug.h"
#include "kernel/memory/kSlabAllocator.h"
#include "kernel/memory/kMemoryPage.h"
#include "kernel/mmu.h"

#define MAX_PAGES 256

static KMemPage pages[MAX_PAGES] = {};

void PageAllocator_Init();
int32_t PageAllocator_AllocPages(size_t size);
int32_t PageAllocator_FindFirstFreePage();
void PageAllocator_FreePages(uint32_t index, size_t size);

uintptr_t PageAllocator_GetPageAddr(uint32_t index);

void PageAllocator_DebugPrintPagesUsed();