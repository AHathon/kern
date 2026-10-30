#include "libc/stdlib.h"
#include "libc/string.h"
#include "libc/assert.h"
#include "libraries/services/svc.h"
#include "kernel/memory/kMemoryMap.h"

#define HEAP_GROW   0x100000UL
#define PAGE        0x1000UL

typedef struct Block
{
    size_t size;
    struct Block *next;
} Block;

static Block *freeList = 0;
static uintptr_t heapEnd = USERLAND_HEAP_BASE;

void free(void *ptr)
{
    if(!ptr)
        return;

    Block *b = (Block*)ptr - 1;
    Block *prev = 0;
    Block *cur = freeList;
    while(cur && cur < b)
    {
        prev = cur;
        cur = cur->next;
    }

    b->next = cur;
    if(prev)
        prev->next = b;
    else
        freeList = b;

    if(cur && (uint8_t*)(b + 1) + b->size == (uint8_t*)cur)
    {
        b->size += sizeof(Block) + cur->size;
        b->next = cur->next;
    }
    if(prev && (uint8_t*)(prev + 1) + prev->size == (uint8_t*)b)
    {
        prev->size += sizeof(Block) + b->size;
        prev->next = b->next;
    }
}

static int Heap_Grow(size_t need)
{
    size_t size = need + sizeof(Block);
    size = size < HEAP_GROW ? HEAP_GROW : (size + PAGE - 1) & ~(PAGE - 1);
    if(svcMapMem((void*)heapEnd, size) != SVC_OK)
        return -1;

    Block *b = (Block*)heapEnd;
    b->size = size - sizeof(Block);
    heapEnd += size;
    free(b + 1);
    return 0;
}

void *malloc(size_t size)
{
    size = (size + 15) & ~15UL;
    if(!size)
        size = 16;

    while(1)
    {
        Block *prev = 0;
        for(Block *cur = freeList; cur; prev = cur, cur = cur->next)
        {
            if(cur->size < size)
                continue;

            if(cur->size >= size + sizeof(Block) + 16)
            {
                Block *rest = (Block*)((uint8_t*)(cur + 1) + size);
                rest->size = cur->size - size - sizeof(Block);
                rest->next = cur->next;
                cur->size = size;
                cur->next = rest;
            }

            if(prev)
                prev->next = cur->next;
            else
                freeList = cur->next;
            return cur + 1;
        }

        if(Heap_Grow(size) != 0)
            return 0;
    }
}

void *calloc(size_t count, size_t size)
{
    size_t total = count * size;
    void *p = malloc(total);
    if(p)
        memset(p, 0, total);
    return p;
}

void *realloc(void *ptr, size_t size)
{
    if(!ptr)
        return malloc(size);

    size_t old = ((Block*)ptr - 1)->size;
    if(size <= old)
        return ptr;

    void *p = malloc(size);
    if(p)
    {
        memcpy(p, ptr, old);
        free(ptr);
    }
    return p;
}

void qsort(void *base, size_t count, size_t size, int (*compare)(const void *, const void *))
{
    uint8_t *arr = base;
    for(size_t gap = count / 2; gap > 0; gap /= 2)
    {
        for(size_t i = gap; i < count; i++)
        {
            for(size_t j = i; j >= gap && compare(arr + (j - gap) * size, arr + j * size) > 0; j -= gap)
            {
                uint8_t *a = arr + (j - gap) * size;
                uint8_t *b = arr + j * size;
                for(size_t k = 0; k < size; k++)
                {
                    uint8_t t = a[k];
                    a[k] = b[k];
                    b[k] = t;
                }
            }
        }
    }
}

int atoi(const char *s)
{
    while(*s == ' ' || *s == '\t')
        s++;

    int neg = *s == '-';
    if(*s == '-' || *s == '+')
        s++;

    int v = 0;
    for(; *s >= '0' && *s <= '9'; s++)
        v = v * 10 + (*s - '0');
    return neg ? -v : v;
}

int abs(int v)
{
    return v < 0 ? -v : v;
}

void __assert_fail(const char *expr, const char *file)
{
    svcDebug("assert failed:");
    svcDebug(expr);
    svcDebug(file);
    *(volatile int*)0 = 0;
    while(1);
}
