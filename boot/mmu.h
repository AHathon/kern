#pragma once

#include "libraries/hardware/uart.h"
#include "libraries/hardware/debug.h"
#include "libraries/hardware/mmu.h"
#include "libraries/hardware/mapping.h"
#include "libraries/hardware/mmio_vars.h"

#define PAGE_SIZE 4096
#define PAGE_TABLE_SIZE (PAGE_SIZE / sizeof(unsigned long))
#define PAGE_TABLE_IDX(i, o) (i * PAGE_TABLE_SIZE + o)

extern volatile unsigned char __rodata_start;
extern volatile unsigned char __page_table;
extern volatile unsigned char __bss_start;

void VMM_Init(void);