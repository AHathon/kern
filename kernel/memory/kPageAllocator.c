#include "kernel/memory/kPageAllocator.h"

void PageAllocator_Init() 
{
    kprintf("Total pages: %d\n", MAX_PAGES);
    for(int i = 0; i < MAX_PAGES / 8; i++)
        pageBitmap[i] = 0;
    kprintf("Initialized kPageAllocator\n");
}

uint8_t PageAllocator_GetPageStatus(uint64_t page) 
{
    return (pageBitmap[page / 8] >> (page % 8)) & 1;
}

void PageAllocator_SetPageUsed(uint64_t page)
{
    pageBitmap[page / 8] |= 1 << (page % 8);
}

void PageAllocator_SetPageFree(uint64_t page)
{
    pageBitmap[page / 8] &= ~(1 << (page % 8));
}

int64_t PageAllocator_FindFirstFreePage(size_t pageCnt) 
{
    int64_t fpage = -1;
    size_t contig = 0;
    for(int i = 0; i < MAX_PAGES/8; i++) 
	{
        for (int j = 0; j < 8; j++)
            if (!((pageBitmap[i] >> j) & 1))
            {
                if(fpage < 0)
                    fpage = ((i * 8) + j);
                if(contig++ == pageCnt)
                    return fpage;
            }
            else 
            {
                contig = 0;
                fpage = -1;
            }
    }

    return -1;
}

int64_t PageAllocator_AllocPages(size_t pageCnt)
{
    uint64_t count = PageAllocator_UsedPagesCount();
    if (pageCnt + count > MAX_PAGES) return -1;

    int64_t firstPage = PageAllocator_FindFirstFreePage(pageCnt);
    if(firstPage < 0) return -1;
    
    // Mark pages as used
    for(size_t i = 0; i < pageCnt; i++)
        PageAllocator_SetPageUsed(firstPage + i);

    return firstPage;
}

void PageAllocator_FreePages(uint32_t page, size_t count)
{
    if (page + count >= MAX_PAGES || count <= 0) return;
    
    for(size_t i = 0; i < count; i++) 
    {
        PageAllocator_SetPageFree(page + i);
    }
}

uint64_t PageAllocator_UsedPagesCount()
{
    uint32_t *page32 = (uint32_t*)pageBitmap;
    uint64_t count = 0;
    for(int i = 0; i < MAX_PAGES / 32; i++)
    {
        //collapsing sums series
        uint32_t x = page32[i];
        x = (x >> 1 & 0x55555555) + (x & 0x55555555);
        x = (x >> 2 & 0x33333333) + (x & 0x33333333);
        x = (x >> 4 & 0x0f0f0f0f) + (x & 0x0f0f0f0f);
        x = (x >> 8 & 0x00ff00ff) + (x & 0x00ff00ff);
        x = (x >> 16) + (x & 0x0000ffff);
        count += x;
    }

    return count;
}

void PageAllocator_DebugPrintPagesUsed() 
{
    kprintf("Pages in use:\n");
    for(int i = 0; i < MAX_PAGES; i++) 
	{
        if(PageAllocator_GetPageStatus(i)) 
			kprintf("%d, ", i);
    }
    kprintf("\n");
    kprintf("Total pages: %d\n", PageAllocator_UsedPagesCount());
}