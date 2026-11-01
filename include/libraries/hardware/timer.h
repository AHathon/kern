#pragma once

#include "libraries/hardware/mmio.h"
#include "libraries/types.h"

void resetSystemTimers();
uint64_t GetCounterFreq();
void localTimerIrqReset();
void localTimerIrqSet(uint64_t ticks);
uint64_t localTimerTicks();
void localTimerIrqInit();