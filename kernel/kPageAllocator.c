#include "kernel/kPageAllocator.h"

void PageAllocator_Init() {
	kprintf("Total pages: %d\n", MAX_PAGES);
    for(int i = 0; i < MAX_PAGES; i++){
        pages[i].vaddr = i * PAGESIZE;
        pages[i].used = 0;
    }
	kprintf("Initialized kPageAllocator\n");
}

int FindFreePages(uint32_t pageCnt) {
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

void *AllocPages(size_t size) {
    size_t numPages = (size / PAGESIZE) + 1;
	kprintf("Allocating %d pages\n", numPages);
    int ind = FindFreePages(numPages);
	if(ind < 0) return (void*)0;
	
	for(size_t i = 0; i < numPages; i++){
		pages[ind + i].used = 1;
		pages[ind + i].prev = (i == 0) ? (KMemPage*)0 : &pages[ind + i - 1];
		pages[ind + i].next = ((ind + i + 1) < MAX_PAGES) && (i == numPages - 1) ? (KMemPage*)0 : &pages[ind + i + 1];
	}
    return (void*)pages[ind].vaddr;
}

void FreePages(void *ptr) {
	int i = 0;
	int found = 0;
	for(i = 0; i < MAX_PAGES; i++){
		if(ptr == pages[i].vaddr){
			found = 1;
			break;
		}
	}
	kprintf("Found page at %d\n", i);
	if(found){
		while(i < MAX_PAGES){
			kprintf("Freeing page %d\n", i);
			pages[i].used = 0;
			pages[i].next = pages[i].prev = (KMemPage*)0;
			if(pages[++i].prev == (KMemPage*)0) break;
		}
	}
}

void DebugPrintPagesUsed() {
	kprintf("Pages in use:\n");
	for(int i = 0; i < MAX_PAGES; i++){
        if(pages[i].used) kprintf("%d, ", i);
    }
	kprintf("\n");
}