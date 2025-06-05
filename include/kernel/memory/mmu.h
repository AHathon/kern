#pragma once

#include "libraries/hardware/uart.h"
#include "libraries/hardware/debug.h"
#include "libraries/hardware/mmu.h"
#include "libraries/hardware/constants.h"
#include "kernel/memory/kMemoryManager.h"

#define PAGE_TABLE_SIZE (PAGE_SIZE / sizeof(unsigned long))
#define PAGE_TABLE_IDX(i, o) (i * PAGE_TABLE_SIZE + o)

extern volatile unsigned char __text_start;
extern volatile unsigned char __text_end;
extern volatile unsigned char __data_start;
extern volatile unsigned char __bss_start;
extern volatile unsigned char __bss_end;
extern volatile unsigned char __page_table;
extern volatile unsigned char __kips_start;
extern volatile unsigned char __kips_end;

void MMU_SetupVirtKernelSpace();
void MMU_MapMemPages(uintptr_t pageTable, uintptr_t paddr, uintptr_t vaddr, size_t size, uint8_t isKernelMem);
void MMU_MapMemBlocks(uintptr_t pageTable, uintptr_t paddr, uintptr_t vaddr, size_t size, uint8_t isKernelMem);
void MMU_ClearIdentityMap();
void MMU_SetTtrb0(uintptr_t pageTable);