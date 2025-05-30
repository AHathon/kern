#include "kernel/memory/mmu.h"

static uintptr_t pageTable;

inline void MMU_SetupVirtKernelSpace(unsigned long page_table) 
{
    pageTable = page_table;
    unsigned long *paging = (unsigned long*)(page_table);
    unsigned long data_page = ((unsigned long)&__data_start) / PAGE_SIZE;
    unsigned long bss_page = ((unsigned long)&__bss_start) / PAGE_SIZE;
    unsigned long bss_end = ((unsigned long)&__bss_end) / PAGE_SIZE;
    unsigned long text_end_page = ((unsigned long)&__text_end) / PAGE_SIZE;
    uint32_t r;

    // TTBR1, kernel L1
    paging[PAGE_TABLE_IDX(1, 0)] = (unsigned long)((unsigned char *)page_table + 4 * PAGE_SIZE) |
        PT_TABLE | 
        PT_AF | 
        PT_KERNEL | 
        PT_ISH | 
        PT_MEM;

    for (r = 1; r < PAGE_TABLE_SIZE; r++)
    {
        uint32_t flags = PT_ISH | PT_MEM;
        if(r >= (MMIO_PADDR >> 30) & 0x1FF)
            flags = PT_OSH | PT_DEV;
        paging[PAGE_TABLE_IDX(1, r)] = (r << 30) |
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
        uint32_t flags = PT_ISH | PT_MEM;
        if(r >= (MMIO_PADDR >> 21) & 0x1FF)
            flags = PT_OSH | PT_DEV;
        paging[PAGE_TABLE_IDX(4, r)] = (r << 21) |
            PT_BLOCK | 
            PT_AF | 
            PT_NX | 
            PT_KERNEL | 
            flags;
    }

    // Overwrite .bss as readable/writable memory
    // Kernel L3: map a single page (e.g., MMIO or kernel stack)
    for (r = 0; r < PAGE_TABLE_SIZE; r++) 
        paging[PAGE_TABLE_IDX(5, r)] = (r * PAGE_SIZE) |
            PT_PAGE | 
            PT_AF |
            PT_KERNEL | 
            PT_ISH |
            PT_RW;

    MMIO_BASE = MMIO_PADDR + KERNEL_VIRT_BASE;
}

void MMU_mapMem(uintptr_t paddr, uintptr_t vaddr)
{
    unsigned long *paging = (unsigned long*)(pageTable);
    //TODO
}