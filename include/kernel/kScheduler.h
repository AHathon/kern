#pragma once

#include "kernel/kProcessManager.h"
#include "libraries/hardware/irq.h"

static unsigned currProc;

void kScheduler_init();
void context_switch();