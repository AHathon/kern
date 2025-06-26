#pragma once

#include "libraries/hardware/uart.h"
#include "libraries/hardware/debug.h"
#include "libraries/hardware/mmu.h"
#include "libraries/hardware/constants.h"
#include "libraries/hardware/mmio_vars.h"

extern volatile unsigned char __text_start;
extern volatile unsigned char __kernel_blob;
extern volatile unsigned char __rodata_start;
extern volatile unsigned char __data_start;
extern volatile unsigned char __bss_start;
extern volatile unsigned char __page_table;
extern volatile unsigned char __page_table_end;

void VMM_Init(void);