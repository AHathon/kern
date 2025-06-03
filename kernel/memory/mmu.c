#include "kernel/memory/mmu.h"

static uintptr_t pageTable;

inline void MMU_SetupVirtKernelSpace(unsigned long page_table) 
{
    pageTable = page_table;
    unsigned long *paging = (unsigned long*)(pageTable);
    unsigned long data_page = ((unsigned long)&__data_start) / PAGE_SIZE;
    unsigned long bss_page = ((unsigned long)&__bss_start) / PAGE_SIZE;
    unsigned long bss_end = ((unsigned long)&__bss_end) / PAGE_SIZE;
    unsigned long text_end_page = ((unsigned long)&__text_end) / PAGE_SIZE;
    unsigned long kips = ((unsigned long)&__kips_start) / PAGE_SIZE;
    unsigned long kips_end = ((unsigned long)&__kips_end) / PAGE_SIZE;
    uint64_t r;
    UNUSED(data_page);
    UNUSED(bss_end);
    UNUSED(text_end_page);

    // TTBR1, kernel L1
    paging[PAGE_TABLE_IDX(1, 0)] = (unsigned long)((unsigned char *)page_table + 4 * PAGE_SIZE) |
        PT_TABLE | 
        PT_AF | 
        PT_KERNEL | 
        PT_ISH | 
        PT_MEM;

    for (r = 1; r < PAGE_TABLE_SIZE; r++)
    {
        uint64_t flags = PT_ISH | PT_MEM;
        if(r == L1_IDX(MMIO_BASE) ||
            r == L1_IDX(GIC_BASE) ||
            r == L1_IDX(ARM_LOCAL_BASE))
                flags = PT_OSH | PT_DEV;
        paging[PAGE_TABLE_IDX(1, r)] = (r << L1_SHIFT) |
            PT_BLOCK | 
            PT_AF | 
            PT_NX | 
            PT_KERNEL | 
            flags;
    }

    // Kernel L2
    paging[PAGE_TABLE_IDX(4, 0)] = (unsigned long)((unsigned char *)page_table + 5 * PAGE_SIZE) |
        PT_TABLE | 
        PT_AF | 
        PT_KERNEL | 
        PT_ISH | 
        PT_MEM;
    
    for (r = 1; r < PAGE_TABLE_SIZE; r++)
    {
        uint64_t flags = PT_ISH | PT_MEM;
        if(r == L2_IDX(MMIO_BASE) ||
            r == L2_IDX(GIC_BASE) ||
            r == L2_IDX(ARM_LOCAL_BASE))
                flags = PT_OSH | PT_DEV;
        paging[PAGE_TABLE_IDX(4, r)] = (r << L2_SHIFT) |
            PT_BLOCK | 
            PT_AF | 
            PT_NX | 
            PT_KERNEL | 
            flags;
    }

    // Kernel L3
    for (r = 0; r < PAGE_TABLE_SIZE; r++) 
    {
        uint64_t flags = PT_MEM | PT_RW;
        if(r >= kips && r < kips_end)
            flags = PT_MEM | PT_RO;
        paging[PAGE_TABLE_IDX(5, r)] = (r << L3_SHIFT) |
            PT_PAGE | 
            PT_AF |
            PT_KERNEL | 
            PT_ISH |
            flags;
    }

    MMIO_ADDR = MMIO_BASE + KERNEL_VIRT_BASE;
    GICC_ADDR = GIC_BASE + KERNEL_VIRT_BASE;
    ARM_LOCAL_ADDR = ARM_LOCAL_BASE + KERNEL_VIRT_BASE;
    asm volatile("dsb sy"); 
}

void MMU_ClearIdentityMap()
{
    uint64_t *paging = (uint64_t*)(pageTable + KERNEL_VIRT_BASE);
    paging[PAGE_TABLE_IDX(0, 0)] = 0;

    //Flush TBL, sync and jump
    uintptr_t kern_start = KERNEL_VIRT_BASE + (uintptr_t)&__text_start;
    asm volatile(
        "mov x8, %0\n"
        "mov x0, #0\n"
        "mov x1, #1\n"
        "br x8\n"
        :
        : "r"(kern_start)
        : "x0", "x1", "x8"
    );
    __builtin_unreachable();
}

void MMU_mapMem(uintptr_t paddr, uintptr_t vaddr)
{
    unsigned long *paging = (unsigned long*)(pageTable + KERNEL_VIRT_BASE);
    //TODO
    UNUSED(paging);
}