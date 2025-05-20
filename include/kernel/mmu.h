#pragma once

#include "kernel/uart.h"
#include "debug.h"

#define KERNEL_VIRT_BASE 0xFFFFFF8000000000

#define PAGE_SIZE 4096
#define PAGE_TABLE_SIZE (PAGE_SIZE / sizeof(unsigned long))
#define PAGE_TABLE_IDX(i, o) (i * PAGE_TABLE_SIZE + o)

#define PT_PAGE     0b11
#define PT_BLOCK    0b01
#define PT_TABLE    0b11

#define PT_KERNEL   (0 << 6)     // EL1 access level
#define PT_USER     (1 << 6)     // EL0 access level
#define PT_RW       (0 << 7)     // read-write access
#define PT_RO       (1 << 7)     // read only
#define PT_AF       (1 << 10)    // access flag
#define PT_NX       (1UL << 54)  // no execute

#define PT_OSH      (2 << 8)  // outer shareable
#define PT_ISH      (3 << 8)  // inner shareable

#define PT_MEM      (0 << 2)  // normal memory
#define PT_DEV      (1 << 2)  // MMIO
#define PT_NC       (2 << 2)  // non-cacheable

#define TTBR_CNP 1

extern volatile unsigned char __data_start;
extern volatile unsigned char __page_table;
extern volatile unsigned char __bss_start;

void VMM_Init(void);