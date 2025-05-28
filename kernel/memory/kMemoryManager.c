#include "kernel/memory/kMemoryManager.h"

void kMemManager_Init()
{
    PageAllocator_Init();
    kprintf("Initialized kMemoryManager\n");
}

void *kMemAlloc(size_t size)
{
    size_t pageCnt = (size + PAGE_SIZE - 1) / PAGE_SIZE;
	int64_t page = PageAllocator_AllocPages(pageCnt);
    return (void*)(page * PAGE_SIZE);
}

void kMemFree(void *ptr, size_t size)
{
    size_t pageCnt = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    int64_t page = (uintptr_t)ptr / PAGE_SIZE;
    PageAllocator_FreePages(page, pageCnt);
}