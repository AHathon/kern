#pragma once

#include "libraries/types.h"
#include "libraries/hardware/mapping.h"

extern uint64_t MMIO_BASE;

#define GPFSEL0         ((volatile unsigned int*)(GPIO_BASE+0x0000))
#define GPFSEL1         ((volatile unsigned int*)(GPIO_BASE+0x0004))
#define GPFSEL2         ((volatile unsigned int*)(GPIO_BASE+0x0008))
#define GPFSEL3         ((volatile unsigned int*)(GPIO_BASE+0x000C))
#define GPFSEL4         ((volatile unsigned int*)(GPIO_BASE+0x0010))
#define GPFSEL5         ((volatile unsigned int*)(GPIO_BASE+0x0014))
#define GPSET0          ((volatile unsigned int*)(GPIO_BASE+0x001C))
#define GPSET1          ((volatile unsigned int*)(GPIO_BASE+0x0020))
#define GPCLR0          ((volatile unsigned int*)(GPIO_BASE+0x0028))
#define GPLEV0          ((volatile unsigned int*)(GPIO_BASE+0x0034))
#define GPLEV1          ((volatile unsigned int*)(GPIO_BASE+0x0038))
#define GPEDS0          ((volatile unsigned int*)(GPIO_BASE+0x0040))
#define GPEDS1          ((volatile unsigned int*)(GPIO_BASE+0x0044))
#define GPHEN0          ((volatile unsigned int*)(GPIO_BASE+0x0064))
#define GPHEN1          ((volatile unsigned int*)(GPIO_BASE+0x0068))
#define GPPUD           ((volatile unsigned int*)(GPIO_BASE+0x0094))
#define GPPUDCLK0       ((volatile unsigned int*)(GPIO_BASE+0x0098))
#define GPPUDCLK1       ((volatile unsigned int*)(GPIO_BASE+0x009C))