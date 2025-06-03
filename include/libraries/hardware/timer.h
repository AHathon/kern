#pragma once

#include "libraries/hardware/mmio.h"
#include "libraries/types.h"

void resetSystemTimers();
uint64_t GetCounterFreq();
void localTimerIrqInit();