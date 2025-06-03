#pragma once

#include "libraries/hardware/uart.h"
#include "libraries/hardware/debug.h"
#include "libraries/hardware/mmu.h"
#include "libraries/hardware/constants.h"

#define PAGE_SIZE 4096
#define PAGE_TABLE_SIZE (PAGE_SIZE / sizeof(unsigned long))
#define PAGE_TABLE_IDX(i, o) (i * PAGE_TABLE_SIZE + o)

extern volatile unsigned char __data_start;
extern volatile unsigned char __bss_start;
extern volatile unsigned char __bss_end;
extern volatile unsigned char __text_start;
extern volatile unsigned char __text_end;
extern volatile unsigned char __page_table;
extern volatile unsigned char __kips_start;
extern volatile unsigned char __kips_end;

void MMU_SetupVirtKernelSpace(unsigned long page_table);
void MMU_mapMem(uintptr_t paddr, uintptr_t vaddr);
void MMU_ClearIdentityMap();