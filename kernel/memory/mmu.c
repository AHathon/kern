#include "kernel/memory/mmu.h"
#include "kernel/memory/kPageAllocator.h"

inline void MMU_SetupVirtKernelSpace() 
{
    unsigned long *paging = (unsigned long*)(&__page_table);
    unsigned long data_page = ((unsigned long)&__data_start) / PAGE_SIZE;
    unsigned long bss_page = ((unsigned long)&__bss_start) / PAGE_SIZE;
    unsigned long bss_end = ((unsigned long)&__bss_end) / PAGE_SIZE;
    unsigned long text_end_page = ((unsigned long)&__text_end) / PAGE_SIZE;
    unsigned long kips = ((unsigned long)&__kips_start) / PAGE_SIZE;
    unsigned long kips_end = ((unsigned long)&__kips_end) / PAGE_SIZE;
    uint64_t r;
    UNUSED(data_page);
    UNUSED(bss_page);
    UNUSED(bss_end);
    UNUSED(text_end_page);

    // TTBR1, kernel L1
    paging[PAGE_TABLE_IDX(0, 0)] = (unsigned long)((unsigned char *)&__page_table + 1 * PAGE_SIZE) |
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
        paging[PAGE_TABLE_IDX(0, r)] = (r << L1_SHIFT) |
            PT_BLOCK | 
            PT_AF | 
            PT_UXN | 
            PT_KERNEL | 
            flags;
    }

    // Kernel L2
    paging[PAGE_TABLE_IDX(1, 0)] = (unsigned long)((unsigned char *)&__page_table + 2 * PAGE_SIZE) |
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
        paging[PAGE_TABLE_IDX(1, r)] = (r << L2_SHIFT) |
            PT_BLOCK | 
            PT_AF | 
            PT_UXN | 
            PT_KERNEL | 
            flags;
    }

    // Kernel L3
    for (r = 0; r < PAGE_TABLE_SIZE; r++) 
    {
        uint64_t flags = PT_MEM | PT_RW;
        if(r >= kips && r < kips_end)
            flags = PT_MEM | PT_RO;
        paging[PAGE_TABLE_IDX(2, r)] = (r << L3_SHIFT) |
            PT_PAGE | 
            PT_AF |
            PT_KERNEL | 
            PT_ISH |
            flags;
    }

    MMIO_ADDR = MMIO_BASE + KERNEL_VIRT_BASE;
    GICC_ADDR = GIC_BASE + KERNEL_VIRT_BASE;
    ARM_LOCAL_ADDR = ARM_LOCAL_BASE + KERNEL_VIRT_BASE;

    unsigned long page = (unsigned long)&__page_table;
    asm volatile ("msr ttbr1_el1, %0" : : "r" (page | TTBR_CNP)); // upper half, kernel space (set common-not-priv)
    
    asm volatile("tlbi vmalle1is"); 
    asm volatile("dsb sy"); 
    asm volatile("isb");
}

void MMU_ClearIdentityMap()
{
    //Clear sensitive addresses from identity map
    uint64_t ttbr0;
    asm volatile("mrs %0, ttbr0_el1" : "=r"(ttbr0));
    ttbr0 &= PHYS_ADDR_MASK;

    uint64_t start = (uint64_t)&__text_start;
    uint64_t end = (uint64_t)&__text_end;
    MMU_UnmapMemPages(ttbr0, KERN_VADDR_TO_PADDR(start), end - start);
}

void MMU_MapMemPages(uintptr_t pageTable, uintptr_t paddr, uintptr_t vaddr, size_t size, uint8_t isKernelMem)
{
    LOG("Mapping mem @ 0x%X(0x%X) [0x%X bytes]\n", paddr, vaddr, size);
    size_t pageCnt = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    for(int i = 0; i < pageCnt; i++)
    {        
        uintptr_t va = (vaddr + i * PAGE_SIZE);
        uintptr_t pa = paddr + i * PAGE_SIZE;

        uint64_t L1_entry = 0;
        uint64_t L2_entry = 0;
        uint64_t L3_entry = 0;

        uint64_t *L1_tbl_ptr = 0;
        uint64_t *L2_tbl_ptr = 0;
        uint64_t *L3_tbl_ptr = 0;

        uint64_t comm_flags = (isKernelMem ? PT_KERNEL : PT_USER);

        //L1        
        L1_tbl_ptr = (uint64_t*)(pageTable);
        L1_entry = L1_tbl_ptr[L1_IDX(va)];

        uint64_t L1_type = (L1_entry & 0b11);
        if(L1_type != PT_TABLE)
        {
            //Allocate table
            L2_tbl_ptr = (uint64_t*)kMemCalloc(PAGE_SIZE);
            L2_entry = KERN_VADDR_TO_PADDR((uint64_t)L2_tbl_ptr);
            L1_tbl_ptr[L1_IDX(va)] = (L2_entry) |
                PT_TABLE | 
                PT_AF | 
                PT_ISH |
                comm_flags;
        }
        else
        {
            //Has table, so lets extract addr
            L2_tbl_ptr = (uint64_t*)((L1_entry & PHYS_ADDR_MASK) + KERNEL_VIRT_BASE);
            L2_entry = L2_tbl_ptr[L2_IDX(va)];
        }

        //L2
        uint64_t L2_type = (L2_entry & 0b11);
        if(L2_type != PT_TABLE)
        {
            L3_tbl_ptr = (uint64_t*)kMemCalloc(PAGE_SIZE);
            L3_entry = KERN_VADDR_TO_PADDR((uint64_t)L3_tbl_ptr);
            L2_tbl_ptr[L2_IDX(va)] = (L3_entry) |
                PT_TABLE | 
                PT_AF | 
                PT_ISH |
                comm_flags;
        }
        else
        {
            L3_tbl_ptr = (uint64_t*)((L2_entry & PHYS_ADDR_MASK) + KERNEL_VIRT_BASE);
            L3_entry = L3_tbl_ptr[L3_IDX(va)];
        }

        //L3
        L3_tbl_ptr[L3_IDX(va)] = (pa & ~((1UL << L3_SHIFT) - 1)) |
            PT_PAGE | 
            PT_AF |
            PT_ISH |
            comm_flags;
    }

    asm volatile("tlbi vmalle1is"); 
    asm volatile("dsb sy"); 
    asm volatile("isb");
}

void MMU_MapMemBlocks(uintptr_t pageTable, uintptr_t paddr, uintptr_t vaddr, size_t size, uint8_t isKernelMem)
{
    size_t remaining_size = size;
    uint64_t curr_vaddr = vaddr;
    uint64_t curr_paddr = paddr;
    UNUSED(curr_paddr);
    UNUSED(curr_vaddr);

    //Blocks are 2MB min
    if(size < MB2_SIZE) return;

    while(remaining_size > 0)
    {
        //TODO: the rest

        remaining_size -= PAGE_SIZE;
        curr_vaddr += PAGE_SIZE;
        curr_paddr += PAGE_SIZE;
    }
}

void MMU_UnmapMemPages(uintptr_t pageTable, uintptr_t vaddr, size_t size)
{
    LOG("Unmapping mem @ 0x%X [0x%X bytes]\n", vaddr, size);
    size_t pageCnt = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    uint64_t L1_entry = 0;
    uint64_t L2_entry = 0;
    uint64_t L3_entry = 0;

    uint64_t *L1_tbl_ptr = 0;
    uint64_t *L2_tbl_ptr = 0;
    uint64_t *L3_tbl_ptr = 0;

    uintptr_t va = 0;

    L1_tbl_ptr = (uint64_t*)(pageTable);
    if(!L1_tbl_ptr)
    {
        ERROR("Bad page table?\n");
        return;
    }

    for(int i = 0; i < pageCnt; i++)
    {
        va = vaddr + (i * PAGE_SIZE);

        //L1
        L1_entry = L1_tbl_ptr[L1_IDX(va)];

        uint64_t L1_type = (L1_entry & 0b11);
        L2_tbl_ptr = 0;
        L2_entry = 0;
        if(L1_type == PT_TABLE)
        {
            L2_tbl_ptr = (uint64_t*)((L1_entry & PHYS_ADDR_MASK) + KERNEL_VIRT_BASE);
            L2_entry = L2_tbl_ptr[L2_IDX(va)];
        }
        else
        {
            ERROR("L1 type not table @ 0x%X\n", va);
            continue;
        }

        //L2
        uint64_t L2_type = (L2_entry & 0b11);
        L3_tbl_ptr = 0;
        L3_entry = 0;
        if(L2_type == PT_TABLE)
        {
            L3_tbl_ptr = (uint64_t*)((L2_entry & PHYS_ADDR_MASK) + KERNEL_VIRT_BASE);
            L3_entry = L3_tbl_ptr[L3_IDX(va)];
        }
        else
        {
            ERROR("L2 type not table @ 0x%X\n", va);
            continue;
        }

        //L3
        uint64_t L3_type = (L3_entry & 0b11);
        if(L3_type == PT_PAGE)
        {
            L3_tbl_ptr[L3_IDX(va)] = 0;
            asm volatile("dsb ishst");
            asm volatile("tlbi vaae1is, %0" :: "r" (va));
            asm volatile("dsb ish");
            asm volatile("isb");
        }
        else
        {
            ERROR("L3 type not page @ 0x%X\n", va);
            continue;
        }
    }
}

void MMU_SetTtrb0(uintptr_t pageTable)
{
    asm volatile ("msr ttbr0_el1, %0" :: "r" (pageTable));
}

uint64_t *MMU_GetKernelPageTable()
{
    return (uint64_t*)(&__page_table);
}