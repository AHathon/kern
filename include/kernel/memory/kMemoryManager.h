#pragma once

#include <stddef.h>

void kMemManager_Init();
void *KMemAlloc(size_t size);
void kMemFree(void *size);