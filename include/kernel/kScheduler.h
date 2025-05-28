#pragma once

#include "kernel/kProcessManager.h"
#include "libraries/hardware/irq.h"

static unsigned currThread;

void kScheduler_init();
void context_switch();