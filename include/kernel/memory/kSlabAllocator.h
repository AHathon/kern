#pragma once

#include "libraries/types.h"
#include "kernel/memory/kMemoryPage.h"
#include "kernel/memory/kPageAllocator.h"

typedef struct
{
    struct slab_node_t *next;
} slab_node_t;

typedef struct
{
    void *memPtr;
    size_t objSize;
    size_t objCnt;
    slab_node_t *head;
} slab_allocator_t;

void kSlabAlloc_Init(slab_allocator_t *slabAlloc, size_t objSize, size_t objCnt);
slab_node_t *kSlabAlloc_Allocate(slab_allocator_t *slabAlloc);
void kSlabAlloc_Free(slab_allocator_t *slabAlloc, slab_node_t *slab);
void kSlabAlloc_Destroy(slab_allocator_t *slabAlloc);