#pragma once

#include "libraries/types.h"
#include "kernel/memory/kMemoryPage.h"

#define KERN_HEAP_START 0xC0000000

typedef struct {
	unsigned pagesActive;
	KMemPage *pageList;
} kSlab;

void kSlabInit(unsigned long size);
