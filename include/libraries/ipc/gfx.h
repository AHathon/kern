#pragma once

#include "libraries/types.h"
#include "libraries/ipc/service.h"

#define GFX_OK              0
#define GFX_ERR_INVALID     -200
#define GFX_ERR_NO_MEMORY   -201
#define GFX_ERR_BUSY        -202

typedef struct
{
    float x, y;
    float u, v;
    uint32_t color;     //0xAABBGGRR
} GfxVertex;

typedef struct
{
    int32_t clipX0, clipY0, clipX1, clipY1;
    uint32_t texture;   //0 draws untextured
    uint32_t idxOffset;
    uint32_t vtxOffset;
    uint32_t elemCount;
} GfxDrawCmd;

#ifdef __cplusplus
extern "C" {
#endif

long gfxInitialize(Service *gfx);
void gfxExit(Service *gfx);
long gfxCreateTexture(Service *gfx, uint32_t width, uint32_t height, uint32_t *texture);
long gfxUpdateTexture(Service *gfx, uint32_t texture, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void *pixels, uint32_t pitch);
long gfxDestroyTexture(Service *gfx, uint32_t texture);
long gfxBeginFrame(Service *gfx, uint32_t clearColor);
long gfxDrawList(Service *gfx, const GfxVertex *vertices, uint32_t vtxCount, const uint16_t *indices, uint32_t idxCount, const GfxDrawCmd *cmds, uint32_t cmdCount);
long gfxEndFrame(Service *gfx);

#ifdef __cplusplus
}
#endif
