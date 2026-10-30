#pragma once

#include "libraries/types.h"
#include "gfx.h"

typedef struct
{
    uint32_t width;
    uint32_t height;
    uint32_t *pixels;   //0xAABBGGRR
} RasterTexture;

int Raster_Init(int width, int height);
void Raster_Clear(uint32_t color);
void Raster_DrawCmd(const GfxDrawCmd *cmd, const GfxVertex *vertices, uint32_t vtxCount, const uint16_t *indices, const RasterTexture *tex);
void Raster_Present(void *framebuffer, int pitch);
