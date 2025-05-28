#pragma once

#include "kernel/memory/kPageAllocator.h"

void kMemManager_Init();
void *kMemAlloc(size_t size);
void kMemFree(void *ptr, size_t size);