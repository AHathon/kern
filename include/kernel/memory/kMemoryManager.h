#pragma once

#include <stddef.h>
#include "kernel/kPageAllocator.h"
#include "kernel/memory/kSlabAllocator.h"

static kSlab userHeap;
static kSlab privHeap;

void kMemManager_Init();
void *kMemAlloc(size_t size);
void kMemFree(void *size);