#pragma once

#include "libraries/types.h"

void GICD_SetGroup(uint32_t irq, uint8_t grp);
void GICD_RouteIRQ(uint32_t irq);
void GICD_EnableIRQ(uint32_t irq);
void GICD_DisableIRQ(uint32_t irq);
void GICD_ClearPendingIRQ(uint32_t irq);
void GICD_ClearActiveIRQ(uint32_t irq);

uint32_t GIC_GetGicMaxIRQs();
void GIC_Enable();
void GIC_Disable();