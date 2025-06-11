#include "kernel/memory/kMemoryManager.h"

void kMemManager_Init()
{
    PageAllocator_Init();
    LOGT("Initialized kMemoryManager\n");
}

void *kMemAlloc(size_t size)
{
    size_t pageCnt = (size + PAGE_SIZE - 1) / PAGE_SIZE;
	int64_t page = PageAllocator_AllocPages(pageCnt);
    return (void*)(KERN_PADDR_TO_VADDR(page * PAGE_SIZE));
}

void *kMemCalloc(size_t size)
{
    uint8_t *ptr = kMemAlloc(size);
    kmemset(ptr, size);
    return (void*)ptr;
}

void kMemFree(void *ptr, size_t size)
{
    size_t pageCnt = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    int64_t page = (uintptr_t)(KERN_VADDR_TO_PADDR(ptr)) / PAGE_SIZE;
    PageAllocator_FreePages(page, pageCnt);
}