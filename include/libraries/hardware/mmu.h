#pragma once

#include "libraries/hardware/mmio_vars.h"

#define PT_INVALID  0b00
#define PT_PAGE     0b11
#define PT_BLOCK    0b01
#define PT_TABLE    0b11

#define PT_KERNEL   (0UL << 6)     // EL1 access level
#define PT_USER     (1UL << 6)     // EL0 access level
#define PT_RW       (0UL << 7)     // read-write access
#define PT_RO       (1UL << 7)     // read only
#define PT_AF       (1UL << 10)    // access flag
#define PT_PXN      (1UL << 53)
#define PT_UXN      (1UL << 54)
#define PT_NX       (PT_PXN | PT_UXN)
#define PT_EXEC     (0UL << 53)

#define PT_OSH      (2 << 8)  // outer shareable
#define PT_ISH      (3 << 8)  // inner shareable

#define PT_MEM      (0 << 2)  // normal memory
#define PT_DEV      (1 << 2)  // MMIO
#define PT_NC       (2 << 2)  // non-cacheable

#define L1_SHIFT 30
#define L2_SHIFT 21
#define L3_SHIFT 12
#define L1_IDX(x) ((x >> L1_SHIFT) & 0x1FF)
#define L2_IDX(x) ((x >> L2_SHIFT) & 0x1FF)
#define L3_IDX(x) ((x >> L3_SHIFT) & 0x1FF)

#define PHYS_ADDR_MASK  (~((PAGE_SIZE) - 1ULL))

#define TTBR_CNP 1

#define PAGE_SIZE 4096
#define MB2_SIZE (PAGE_TABLE_SIZE * PAGE_SIZE)
#define GB_SIZE (PAGE_TABLE_SIZE * MB2_SIZE)