#pragma once

#include "debug.h"

void svcMapMem(void *addr, long size);
void svcFreeMem(void *addr);
void svcDebug(void);