#pragma once

#include "kernel/kProcessManager.h"
#include "libraries/hardware/irq.h"

static unsigned currThread;

void kScheduler_Init();
void context_switch();