#pragma once

#include "libraries/hardware/debug.h"
#include "libraries/types.h"
#include "libc/string.h"

#define ASSERT(exp)                 \
    (void) ((exp) || (              \
        LOG("Assert: %s\n", #exp),  \
        panic(),                    \
        0                           \
    ))                              \

#define ALIGN(x) __attribute__((aligned(x)))

