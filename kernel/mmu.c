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
    asm volatile ("msr ttbr0_el1, %0" : : "r" ((unsigned long)&__page_table + TTBR_CNP)); // lower half, user space
    asm volatile ("msr ttbr1_el1, %0" : : "r" ((unsigned long)&__page_table + TTBR_CNP + PAGESIZE)); // upper half, kernel space
    
    //Get system control reg and enable MMU
    asm volatile ("dsb ish; isb; mrs %0, sctlr_el1" : "=r" (r));
    r |= (1 << 0);   //MMU enable;
    asm volatile ("msr sctlr_el1, %0; isb" : : "r" (r));
}

void initPageTables() {
    unsigned long data_page = (unsigned long)&__data_start/PAGESIZE;
    unsigned long *paging=(unsigned long*)&__page_table;
    unsigned b, r;
    
    // TTBR0, identity L1
    paging[0]=(unsigned long)((unsigned char*)&__page_table+2*PAGESIZE) |    // physical address
        PT_PAGE |     // it has the "Present" flag, which must be set, and we have area in it mapped by pages
        PT_AF |       // accessed flag. Without this we're going to have a Data Abort exception
        PT_USER |     // non-privileged
        PT_ISH |      // inner shareable
        PT_MEM;       // normal memory

    // identity L2, first 2M block
    paging[2*512]=(unsigned long)((unsigned char*)&__page_table+3*PAGESIZE) | // physical address
        PT_PAGE |     // we have area in it mapped by pages
        PT_AF |       // accessed flag
        PT_USER |     // non-privileged
        PT_ISH |      // inner shareable
        PT_MEM;       // normal memory

    // identity L2 2M blocks
    b=MMIO_BASE>>21;
    // skip 0th, as we're about to map it by L3
    for(r=1;r<512;r++)
        paging[2*512+r]=(unsigned long)((r<<21)) |  // physical address
        PT_BLOCK |    // map 2M block
        PT_AF |       // accessed flag
        PT_NX |       // no execute
        PT_USER |     // non-privileged
        (r>=b? PT_OSH|PT_DEV : PT_ISH|PT_MEM); // different attributes for device memory

    // identity L3
    for(r=0;r<512;r++)
        paging[3*512+r]=(unsigned long)(r*PAGESIZE) |   //paddr -> [0x00000000 - 0x00200000]
        PT_PAGE |     // map 4k
        PT_AF |       // accessed flag
        PT_USER |
        PT_ISH |      // inner shareable
        ((r<0x80||r>data_page)? PT_RW|PT_NX : PT_RO); // different for code and data
    
    //Overwrite .bss to be mem
    paging[3*512 + 0x84] = (unsigned long)(0x84*PAGESIZE) |  // physical address of .bss section
        PT_PAGE |    // map 4k page
        PT_AF |      // accessed flag
        PT_RW |      // read/write
        PT_KERNEL |  // privileged (or PT_USER for non-privileged)
        PT_ISH |     // inner shareable
        PT_MEM;      // normal memory

    // TTBR1, kernel L1
    paging[512+511]=(unsigned long)((unsigned char*)&__page_table+4*PAGESIZE) | // physical address
        PT_PAGE |     // we have area in it mapped by pages
        PT_AF |       // accessed flag
        PT_KERNEL |   // privileged
        PT_ISH |      // inner shareable
        PT_MEM;       // normal memory

    // kernel L2
    paging[4*512+511]=(unsigned long)((unsigned char*)&__page_table+5*PAGESIZE) |   // physical address
        PT_PAGE |     // we have area in it mapped by pages
        PT_AF |       // accessed flag
        PT_KERNEL |   // privileged
        PT_ISH |      // inner shareable
        PT_MEM;       // normal memory

    // kernel L3
    paging[5*512]=(unsigned long)(MMIO_BASE+0x00201000) |   // physical address
        PT_PAGE |     // map 4k
        PT_AF |       // accessed flag
        PT_NX |       // no execute
        PT_KERNEL |   // privileged
        PT_OSH |      // outter shareable
        PT_DEV;       // device memory

}

void MMU_Init() {
    initPageTables();    
    initMMU();
    kprintf("Initialized MMU\n");
    kprintf("Page table at 0x%X\n", (unsigned long)&__page_table);
}