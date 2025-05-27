#include "kernel/memory/kPageAllocator.h"

void PageAllocator_Init() 
{
    kprintf("Total pages: %d\n", MAX_PAGES);
    for(int i = 0; i < MAX_PAGES; i++) {
        pages[i].vaddr = i * PAGE_SIZE;
        pages[i].used = 0;
        pages[i].next = 0;
        pages[i].prev = 0;
    }
    kprintf("Initialized kPageAllocator\n");
}

int32_t PageAllocator_FindFreePages(uint32_t pageCnt) 
{
    if (pageCnt == 0 || pageCnt > MAX_PAGES) return -1;

    uint32_t conseq = 0;
    for(int i = 0; i < MAX_PAGES; i++) {
        if(!pages[i].used) {
            conseq++;
            if(conseq >= pageCnt) {
                return i - (pageCnt - 1);
            }
        } else {
            conseq = 0;
        }
    }
    return -1;
}

int32_t PageAllocator_FindFirstFreePage() 
{
    for(int i = 0; i < MAX_PAGES; i++) 
	{
        if(!pages[i].used)
            return i;
    }
    return -1;
}

int32_t PageAllocator_AllocPages(size_t size) 
{
    if (size == 0) return -1;

    size_t numPages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    if (numPages > MAX_PAGES) return -1;

    kprintf("Allocating %d pages\n", numPages);
    int32_t ind = PageAllocator_FindFreePages(numPages);
    if(ind < 0) return -1;
    
    // Mark pages as used and set up links
    for(size_t i = 0; i < numPages; i++) 
	{
        pages[ind + i].used = 1;
        pages[ind + i].prev = (i == 0) ? 0 : &pages[ind + i - 1];
        pages[ind + i].next = (i == numPages - 1) ? 0 : &pages[ind + i + 1];
    }
    return ind;
}

void PageAllocator_FreePages(uint32_t index, size_t size) 
{
    if (index >= MAX_PAGES || size == 0) return;
    
    size_t numPages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    if (numPages > MAX_PAGES || (index + numPages) > MAX_PAGES) return;

    for(size_t i = 0; i < numPages; i++) {
        if (index + i >= MAX_PAGES) 
			break;
        pages[index + i].used = 0;
        pages[index + i].next = 0;
        pages[index + i].prev = 0;
    }
}

uintptr_t PageAllocator_GetPageAddr(uint32_t index)
{
    if (index >= MAX_PAGES) return 0;
    return pages[index].vaddr;
}

void PageAllocator_DebugPrintPagesUsed() 
{
    kprintf("Pages in use:\n");
    for(int i = 0; i < MAX_PAGES; i++) 
	{
        if(pages[i].used) 
			kprintf("%d, ", i);
    }
    kprintf("\n");
}