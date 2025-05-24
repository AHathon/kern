#pragma once

#include "libraries/types.h"

typedef struct {
    uintptr_t vaddr;
    uint8_t used;
	struct KMemPage *next;
	struct KMemPage *prev;
} KMemPage;