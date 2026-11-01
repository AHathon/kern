#pragma once

#include "driver/host.h"

const UsbHost *Xhci_Init(uintptr_t regs, uint8_t *dma, uint64_t dmaBus, size_t dmaSize);
