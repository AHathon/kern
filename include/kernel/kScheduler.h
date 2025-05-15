#pragma once

#include "kernel/kProcessManager.h"
#include "kernel/irq.h"

static unsigned currProc;

void kScheduler_init();
void context_switch();