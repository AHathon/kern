#include "kernel/mmu.h"

void initMMU() {
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
    asm volatile ("msr ttbr1_el1, %0" : : "r" (paging + PAGESIZE | 
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

void setupUserPageTables() {
    unsigned long *paging = (unsigned long *)&__page_table;
    unsigned long data_page = (unsigned long)&__data_start / PAGESIZE;
    unsigned long bss_page = (unsigned long)&__bss_start / PAGESIZE;
    unsigned b, r;

    // TTBR0, identity L1
    paging[PAGE_TABLE_IDX(0, 0)] = (unsigned long)((unsigned char *)&__page_table + 2 * PAGESIZE) |
        PT_TABLE | 
        PT_AF | 
        PT_USER | 
        PT_ISH | 
        PT_MEM;

    // Identity L2
    paging[PAGE_TABLE_IDX(2, 0)] = (unsigned long)((unsigned char *)&__page_table + 3 * PAGESIZE) |
        PT_TABLE | 
        PT_AF | 
        PT_USER | 
        PT_ISH | 
        PT_MEM;

    // Identity L2 2MB blocks (except first block handled by L3)
    b = MMIO_BASE >> 21;
    for (r = 1; r < PAGE_TABLE_SIZE; r++) {
        paging[PAGE_TABLE_IDX(2, r)] = (r << 21) |
            PT_BLOCK | 
            PT_AF | 
            PT_NX | 
            PT_USER |
            (r >= b ? PT_OSH | PT_DEV : PT_ISH | PT_MEM);
    }

    // Identity L3: map first 2MB region as 4K pages
    for (r = 0; r < PAGE_TABLE_SIZE; r++) {
        paging[PAGE_TABLE_IDX(3, r)] = (r * PAGESIZE) |
            PT_PAGE | 
            PT_AF | 
            PT_USER | 
            PT_ISH |
            ((r < 0x80 || r > data_page) ? PT_RW | PT_NX : PT_RO);
    }

    // Overwrite .bss as readable/writable memory
    paging[PAGE_TABLE_IDX(3, bss_page)] = (unsigned long)(bss_page * PAGESIZE) |
        PT_PAGE | 
        PT_AF | 
        PT_RW | 
        PT_KERNEL | 
        PT_ISH | 
        PT_MEM;
}

void setupKernelPageTables() {
    unsigned long *paging = (unsigned long *)&__page_table;
    unsigned long data_page = (unsigned long)&__data_start / PAGESIZE;
    unsigned long bss_page = (unsigned long)&__bss_start / PAGESIZE;
    unsigned long pageTableSize = PAGESIZE / sizeof(unsigned long);
    unsigned r, b;

    // TTBR1, kernel L1
    paging[PAGE_TABLE_IDX(1, 0)] = (unsigned long)((unsigned char *)&__page_table + 4 * PAGESIZE) |
        PT_TABLE | 
        PT_AF | 
        PT_KERNEL | 
        PT_ISH | 
        PT_MEM;

    // Kernel L2
    paging[PAGE_TABLE_IDX(4, 0)] = (unsigned long)((unsigned char *)&__page_table + 5 * PAGESIZE) |
        PT_TABLE | 
        PT_AF | 
        PT_KERNEL | 
        PT_ISH | 
        PT_MEM;
    b = MMIO_BASE >> 21;
    
    for (r = 1; r < PAGE_TABLE_SIZE; r++)
        paging[PAGE_TABLE_IDX(4, r)] = (r << 21) |
            PT_BLOCK | 
            PT_AF | 
            PT_NX | 
            PT_KERNEL | 
            (r >= b ? PT_OSH | PT_DEV : PT_ISH | PT_MEM);

    // Kernel L3: map a single page (e.g., MMIO or kernel stack)
    for (r = 0; r < PAGE_TABLE_SIZE; r++) 
        paging[PAGE_TABLE_IDX(5, r)] = (r * PAGESIZE) |
            PT_PAGE | 
            PT_AF |
            PT_KERNEL | 
            PT_ISH |
            ((r < 0x80 || r > data_page) ? PT_RW | PT_NX : PT_RO);
}

void MMU_Init() {
    setupUserPageTables();
    setupKernelPageTables();    
    
    initMMU();
    kprintf("Initialized MMU\n");
    kprintf("Page table at 0x%X\n", (unsigned long)&__page_table);
}
