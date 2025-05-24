#pragma once

#include "libraries/hardware/uart.h"
#include "libraries/hardware/debug.h"
#include "libraries/hardware/mmu.h"

#define KERNEL_VIRT_BASE 0xFFFFFF8000000000


#define PAGE_SIZE 4096
#define PAGE_TABLE_SIZE (PAGE_SIZE / sizeof(unsigned long))
#define PAGE_TABLE_IDX(i, o) (i * PAGE_TABLE_SIZE + o)

extern volatile unsigned char __rodata_start;
extern volatile unsigned char __page_table;
extern volatile unsigned char __bss_start;

void VMM_Init(void);