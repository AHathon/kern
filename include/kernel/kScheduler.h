#pragma once

#include "kernel/kProcessManager.h"
#include "libraries/hardware/irq.h"

static unsigned currThread;
static unsigned maxThread;

void kScheduler_Init();
void kScheduler_Start();
void kScheduler_AddThread(kThread thread);
void context_switch();