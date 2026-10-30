#pragma once

#include "libraries/types.h"
#include "libraries/ipc/gfx.h"

#define GFX_PORT_NAME       "gfx"
#define GFX_TRANSFER_SIZE   0x200000

typedef enum
{
    GFX_CMD_ATTACH = 1,
    GFX_CMD_CREATE_TEXTURE,
    GFX_CMD_UPDATE_TEXTURE,
    GFX_CMD_DESTROY_TEXTURE,
    GFX_CMD_BEGIN_FRAME,
    GFX_CMD_DRAW,
    GFX_CMD_END_FRAME
} GfxCommand;

typedef struct
{
    uint32_t width;
    uint32_t height;
} GfxCreateTextureIn;

typedef struct
{
    uint32_t texture;
} GfxTextureRef;

typedef struct
{
    uint32_t texture;
    uint32_t x, y;
    uint32_t width, height;
    uint32_t offset;
} GfxUpdateTextureIn;

typedef struct
{
    uint32_t clearColor;
} GfxBeginFrameIn;

typedef struct
{
    uint32_t vtxOffset, vtxCount;
    uint32_t idxOffset, idxCount;
    uint32_t cmdOffset, cmdCount;
} GfxDrawIn;
