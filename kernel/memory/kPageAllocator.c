#include "kernel/memory/kPageAllocator.h"

void PageAllocator_Init() 
{
	kprintf("Total pages: %d\n", MAX_PAGES);
    for(int i = 0; i < MAX_PAGES; i++){
        pages[i].vaddr = i * PAGE_SIZE;
        pages[i].used = 0;
    }
	kprintf("Initialized kPageAllocator\n");
}

int32_t PageAllocator_FindFreePages(uint32_t pageCnt) 
{
    uint32_t conseq = 0;
	int index = -1;
    for(int i = 0; i < MAX_PAGES; i++){
        if(!pages[i].used) {
            conseq++;
        }
        else {
            conseq = 0;
        }
        if(conseq >= pageCnt) {
			index = i - (pageCnt - 1);
			break;
		}
    }
	
	return index;
}

int32_t PageAllocator_FindFirstFreePage() 
{
	int32_t index = -1;
    for(int i = 0; i < MAX_PAGES; i++){
        if(!pages[i].used) {
            index = i;
			break;
        }
    }
	
	return index;
}

int32_t PageAllocator_AllocPages(size_t size) 
{
    size_t numPages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

	kprintf("Allocating %d pages\n", numPages);
    int32_t ind = PageAllocator_FindFreePages(numPages);
	if(ind < 0) return -1;
	
	for(size_t i = 0; i < numPages; i++){
		pages[ind + i].used = 1;
		pages[ind + i].prev = (i == 0) ? (KMemPage*)0 : &pages[ind + i - 1];
		pages[ind + i].next = ((ind + i + 1) < MAX_PAGES) && (i == numPages - 1) ? (KMemPage*)0 : &pages[ind + i + 1];
	}
    return ind;
}

void PageAllocator_FreePages(uint32_t index, size_t size) 
{	
	size_t numPages = (size + PAGE_SIZE - 1) / PAGE_SIZE;

	uint32_t i = index;
	while(i < numPages){
		pages[i].used = 0;
		pages[i].next = pages[i].prev = (KMemPage*)0;
	}
}

uintptr_t PageAllocator_GetPageAddr(uint32_t index)
{
	return pages[index].vaddr;
}

void PageAllocator_DebugPrintPagesUsed() 
{
	kprintf("Pages in use:\n");
	for(int i = 0; i < MAX_PAGES; i++){
        if(pages[i].used) kprintf("%d, ", i);
    }
	kprintf("\n");
}