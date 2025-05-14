#pragma once

#include "kernel/kProcessManager.h"

static unsigned currProc;

void kScheduler_init();
void kScheduler_add_proc();
void context_switch();