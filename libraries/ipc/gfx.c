#include "libraries/ipc/gfx.h"
#include "libraries/services/svc.h"
#include "libc/string.h"
#include "gfx.h"

#define TRANSFER_VA 0x30000000UL

static uint8_t *transfer = 0;
static size_t transferSize = 0;

long gfxInitialize(Service *gfx)
{
    long r = serviceOpen(gfx, GFX_PORT_NAME);
    if(r != SVC_OK)
        return r;

    long mem = svcCreateSharedMemory(GFX_TRANSFER_SIZE);
    long size = mem < 0 ? mem : svcMapSharedMemory(mem, (void*)TRANSFER_VA);
    if(size < 0)
    {
        serviceClose(gfx);
        return size;
    }

    r = serviceDispatchHandle(gfx, GFX_CMD_ATTACH, 0, 0, mem);
    if(r != GFX_OK)
    {
        serviceClose(gfx);
        return r;
    }

    transfer = (uint8_t*)TRANSFER_VA;
    transferSize = size;
    return GFX_OK;
}

void gfxExit(Service *gfx)
{
    serviceClose(gfx);
}

long gfxCreateTexture(Service *gfx, uint32_t width, uint32_t height, uint32_t *texture)
{
    GfxCreateTextureIn in = { width, height };
    GfxTextureRef out;
    long r = serviceDispatch(gfx, GFX_CMD_CREATE_TEXTURE, &in, sizeof(in), &out, sizeof(out), 0, 0);
    if(r == GFX_OK)
        *texture = out.texture;
    return r;
}

long gfxUpdateTexture(Service *gfx, uint32_t texture, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void *pixels, uint32_t pitch)
{
    size_t rowSize = width * sizeof(uint32_t);
    if(rowSize * height > transferSize)
        return GFX_ERR_INVALID;

    const uint8_t *src = pixels;
    for(uint32_t row = 0; row < height; row++)
        memcpy(transfer + row * rowSize, src + row * pitch, rowSize);

    GfxUpdateTextureIn in = { texture, x, y, width, height, 0 };
    return serviceDispatch(gfx, GFX_CMD_UPDATE_TEXTURE, &in, sizeof(in), 0, 0, 0, 0);
}

long gfxDestroyTexture(Service *gfx, uint32_t texture)
{
    GfxTextureRef in = { texture };
    return serviceDispatch(gfx, GFX_CMD_DESTROY_TEXTURE, &in, sizeof(in), 0, 0, 0, 0);
}

long gfxBeginFrame(Service *gfx, uint32_t clearColor)
{
    GfxBeginFrameIn in = { clearColor };
    return serviceDispatch(gfx, GFX_CMD_BEGIN_FRAME, &in, sizeof(in), 0, 0, 0, 0);
}

long gfxDrawList(Service *gfx, const GfxVertex *vertices, uint32_t vtxCount, const uint16_t *indices, uint32_t idxCount, const GfxDrawCmd *cmds, uint32_t cmdCount)
{
    size_t vtxBytes = vtxCount * sizeof(GfxVertex);
    size_t idxBytes = idxCount * sizeof(uint16_t);
    size_t cmdBytes = cmdCount * sizeof(GfxDrawCmd);
    size_t idxOffset = vtxBytes;
    size_t cmdOffset = (idxOffset + idxBytes + 3) & ~3UL;
    if(cmdOffset + cmdBytes > transferSize)
        return GFX_ERR_INVALID;

    memcpy(transfer, vertices, vtxBytes);
    memcpy(transfer + idxOffset, indices, idxBytes);
    memcpy(transfer + cmdOffset, cmds, cmdBytes);

    GfxDrawIn in = { 0, vtxCount, (uint32_t)idxOffset, idxCount, (uint32_t)cmdOffset, cmdCount };
    return serviceDispatch(gfx, GFX_CMD_DRAW, &in, sizeof(in), 0, 0, 0, 0);
}

long gfxEndFrame(Service *gfx)
{
    return serviceDispatch(gfx, GFX_CMD_END_FRAME, 0, 0, 0, 0, 0, 0);
}
