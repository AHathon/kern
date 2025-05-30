#pragma once

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

#define L1_SHIFT 30
#define L2_SHIFT 21
#define L3_SHIFT 12
#define L1_IDX(x) ((x >> L1_SHIFT) & 0x1FF)
#define L2_IDX(x) ((x >> L2_SHIFT) & 0x1FF)
#define L3_IDX(x) ((x >> L3_SHIFT) & 0x1FF)

#define TTBR_CNP 1