#pragma once

#include "libraries/types.h"

void GicSetGroup(uint32_t irq, uint8_t grp);
void GicRouteIRQ(uint32_t irq);
void GicEnable();
void GicDisable();