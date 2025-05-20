#include "kernel/mmu.h"

inline void setupVMM() {
    unsigned long r, b;
    
    asm volatile ("mrs %0, id_aa64mmfr0_el1" : "=r" (r));
    b = r & 0xF;
    if(r & (0xF << 28) || b < 1) {
        kprintf("ERROR: 4k granule or 36 bit address space not supported\n");
        return;
    }
    
    // set memory attributes array
    r = ((0b11111111 << 0) |   //Attrib0: Normal Memory, Inner+Outer Write-back non-transient
         (0b00000100 << 8) |   //Attrib1: Device-nGnRE memory
         (0b01000100 << 16));  //Attrib2: Normal memory, Inner+Outer Non-Cacheable
    asm volatile ("msr mair_el1, %0" : : "r" (r));
    
    // specify mapping characteristics in translate control register
    r = (0b10 << 30) | // TG1=4k
        (0b11 << 28) |  //Inner shared
        (0b01 << 26) |  //Norm,Outer WB/WA
        (0b01 << 24) |  //Norm,Inner WB/WA
        (25 << 16)   |  //Size offset of mem region addressed by TTBR1_EL1
        (0b00 << 14) |  //TG0=4k
        (0b11 << 12) |  //Inner sharable
        (0b01 << 10) |  //Norm,Outer WB/WA
        (0b01 << 8)  |  //Norm,Inner WB/WA
        25;             //Size offset of mem region addressed by TTBR0_EL1
    asm volatile ("msr tcr_el1, %0; isb" : : "r" (r));
    
    // tell MMU where translation tables are
    unsigned long paging = (unsigned long)&__page_table;
    asm volatile ("msr ttbr0_el1, %0" : : "r" (paging + 0 |
                                                TTBR_CNP)); // lower half, user space (set common-not-priv)
    asm volatile ("msr ttbr1_el1, %0" : : "r" (paging + PAGE_SIZE | 
                                                TTBR_CNP)); // upper half, kernel space (set common-not-priv)
    
    //sync
    asm volatile ("dsb ish; isb");

    //Set mmio to virt addr
    MMIO_BASE += KERNEL_VIRT_BASE;

    //Get system control reg and enable MMU
    asm volatile ("mrs %0, sctlr_el1" : "=r" (r));
    r |= (1 << 0);   //MMU enable;
    asm volatile ("msr sctlr_el1, %0; isb" : : "r" (r));
}

inline void setupUserPageTables() {
    unsigned long *paging = (unsigned long *)&__page_table;
    unsigned long data_page = (unsigned long)&__data_start / PAGE_SIZE;
    unsigned long bss_page = (unsigned long)&__bss_start / PAGE_SIZE;
    uint32_t r;

    // TTBR0, identity L1
    paging[PAGE_TABLE_IDX(0, 0)] = (unsigned long)((unsigned char *)&__page_table + 2 * PAGE_SIZE) |
        PT_TABLE | 
        PT_AF | 
        PT_USER | 
        PT_ISH | 
        PT_MEM;

    // Identity L2
    paging[PAGE_TABLE_IDX(2, 0)] = (unsigned long)((unsigned char *)&__page_table + 3 * PAGE_SIZE) |
        PT_TABLE | 
        PT_AF | 
        PT_USER | 
        PT_ISH | 
        PT_MEM;

    // Identity L2 2MB blocks (except first block handled by L3)
    for (r = 1; r < (MMIO_BASE >> 21); r++) {
        paging[PAGE_TABLE_IDX(2, r)] = (r << 21) |
            PT_BLOCK | 
            PT_AF | 
            PT_NX | 
            PT_USER |
            PT_ISH | PT_MEM;
    }

    // Identity L3: map first 2MB region as 4K pages
    for (r = 0; r < PAGE_TABLE_SIZE; r++) {
        paging[PAGE_TABLE_IDX(3, r)] = (r * PAGE_SIZE) |
            PT_PAGE | 
            PT_AF | 
            PT_USER | 
            PT_ISH |
            ((r < 0x80 || r > data_page) ? PT_RW | PT_NX : PT_RO);
    }

    // Overwrite .bss as readable/writable memory
    paging[PAGE_TABLE_IDX(3, bss_page)] = (unsigned long)(bss_page * PAGE_SIZE) |
        PT_PAGE | 
        PT_AF | 
        PT_RW | 
        PT_KERNEL | 
        PT_ISH | 
        PT_MEM;
}

inline void setupKernelPageTables() {
    unsigned long *paging = (unsigned long *)&__page_table;
    unsigned long data_page = (unsigned long)&__data_start / PAGE_SIZE;
    unsigned long bss_page = (unsigned long)&__bss_start / PAGE_SIZE;
    unsigned long pageTableSize = PAGE_SIZE / sizeof(unsigned long);
    uint32_t r;

    // TTBR1, kernel L1
    paging[PAGE_TABLE_IDX(1, 0)] = (unsigned long)((unsigned char *)&__page_table + 4 * PAGE_SIZE) |
        PT_TABLE | 
        PT_AF | 
        PT_KERNEL | 
        PT_ISH | 
        PT_MEM;

    // Kernel L2
    paging[PAGE_TABLE_IDX(4, 0)] = (unsigned long)((unsigned char *)&__page_table + 5 * PAGE_SIZE) |
        PT_TABLE | 
        PT_AF | 
        PT_KERNEL | 
        PT_ISH | 
        PT_MEM;
    
    for (r = 1; r < PAGE_TABLE_SIZE; r++)
    {
        uint32_t flags = PT_ISH | PT_MEM;
        if(r >= MMIO_BASE >> 21)
            flags = PT_OSH | PT_DEV;
        paging[PAGE_TABLE_IDX(4, r)] = (r << 21) |
            PT_BLOCK | 
            PT_AF | 
            PT_NX | 
            PT_KERNEL | 
            flags;
    }

    // Kernel L3: map a single page (e.g., MMIO or kernel stack)
    for (r = 0; r < PAGE_TABLE_SIZE; r++) 
        paging[PAGE_TABLE_IDX(5, r)] = (r * PAGE_SIZE) |
            PT_PAGE | 
            PT_AF |
            PT_KERNEL | 
            PT_ISH |
            ((r < 0x81 || r > data_page) ? PT_RW | PT_NX : PT_RO);
}

void VMM_Init() {
    kprintf("Setting up VMM/MMU\n");
    setupUserPageTables();
    setupKernelPageTables();    
    
    setupVMM();
}
