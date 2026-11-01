#pragma once

#include "libraries/types.h"

typedef struct
{
    uint32_t width;
    uint32_t height;
    uint32_t pixelOffset;
    uint32_t stride;
    uint32_t bytesPerPixel;
    int redShift, greenShift, blueShift;
    int alphaShift;
    bool topDown;
} BmpInfo;

bool Bmp_ReadInfo(const void *data, size_t size, BmpInfo *info);
void Bmp_Decode(const void *data, const BmpInfo *info, uint32_t *pixels);
