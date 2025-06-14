#pragma once

#include "libraries/types.h"
#include "libraries/hardware/mmio.h"
#include "libraries/hardware/mmio_vars.h"

void GPIO_SetAlt(uint32_t gpio, uint32_t alt);
void GPIO_SetPullNone(uint32_t gpio);
void GPIO_Write(uint32_t gpio, int32_t value);