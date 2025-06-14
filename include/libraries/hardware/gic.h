#pragma once

#include "libraries/types.h"

void GicRouteIRQ(uint32_t irq);
void GicEnable();
void GicDisable();