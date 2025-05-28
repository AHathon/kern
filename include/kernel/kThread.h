#pragma once

#include "libraries/types.h"

#define MAX_THREADS 256

typedef enum
{
    THREAD_USER,
    THREAD_KERNEL
} ThreadType;

typedef enum
{
    FLAG_IS_ACTIVE,
} ThreadFlags;

typedef struct
{
    uintptr_t stack;
    ThreadType threadType;
    uint32_t flags;
} kThread;

static kThread threadList[MAX_THREADS];

kThread kThread_Create();
void kThread_Destroy();