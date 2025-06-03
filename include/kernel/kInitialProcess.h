#pragma once

#include "libraries/types.h"


extern volatile unsigned char __kips_start;
extern volatile unsigned char __kips_end;

typedef struct
{
    char magic[8];
    uint64_t totalSize;
    uint64_t codeSize;
    uint32_t headerSize;
} KipHeader;


void kInitialProcess_Setup();