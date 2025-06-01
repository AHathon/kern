#pragma once

#include "libraries/hardware/mmio.h"
#include "libraries/types.h"

#define TIMER_INTERVAL 1000000  // 1 million µs = 1s

void resetSystemTimers();
uint64_t GetCounterFreq();
inline void localTimerReset();
void localTimerIrqInit();