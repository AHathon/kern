#pragma once

#include "libraries/hardware/uart.h"
#include "libraries/hardware/debug.h"
#include "libraries/hardware/mmu.h"
#include "libraries/hardware/constants.h"
#include "kernel/memory/kMemoryManager.h"

extern unsigned char _start_kernel;

extern volatile unsigned char __text_start;
extern volatile unsigned char __text_end;
extern volatile unsigned char __data_start;
extern volatile unsigned char __bss_start;
extern volatile unsigned char __bss_end;
extern volatile unsigned char __page_table;
extern volatile unsigned char __kips_start;
extern volatile unsigned char __kips_end;

void MMU_SetupVirtKernelSpace();
void MMU_MapMemPages(uintptr_t pageTable, uintptr_t paddr, uintptr_t vaddr, size_t size, MemoryAttribs attrib);
void MMU_MapMemBlocks(uintptr_t pageTable, uintptr_t paddr, uintptr_t vaddr, size_t size, MemoryAttribs attrib);
void MMU_UnmapMemPages(uintptr_t pageTable, uintptr_t vaddr, size_t size);
void MMU_ClearIdentityMap();
void MMU_SetTtrb0(uintptr_t pageTable);
uint64_t *MMU_GetKernelPageTable();