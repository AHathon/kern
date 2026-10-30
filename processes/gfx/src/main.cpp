#include "libraries/ipc/server.h"
#include "libraries/services/svc.h"
#include "libraries/hardware/constants.h"
#include "kernel/memory/kMemoryMap.h"
#include "libraries/libc/stdio.h"
#include "libraries/libc/stdlib.h"
#include "libraries/libc/string.h"
#include "gfx.h"
#include "rasterizer.h"

#define TRANSFER_VA     0x70000000UL
#define MAX_TEXTURES    32
#define MAX_TEXTURE_DIM 4096

static RasterTexture textures[MAX_TEXTURES];
static const uint8_t *transfer = nullptr;
static size_t transferSize = 0;

//Client-supplied offsets; a read past the transfer buffer would fault
static bool TransferRange(uint64_t offset, uint64_t size)
{
    return transfer && offset <= transferSize && size <= transferSize - offset && !(offset & 3);
}

static RasterTexture *GetTexture(uint32_t id)
{
    if(id == 0 || id > MAX_TEXTURES || !textures[id - 1].pixels)
        return nullptr;
    return &textures[id - 1];
}

static long Gfx_Attach(ServerRequest *req)
{
    if(req->copyHandle <= 0)
        return GFX_ERR_INVALID;

    if(transfer)
    {
        svcCloseHandle(req->copyHandle);
        return GFX_ERR_BUSY;
    }

    long size = svcMapSharedMemory(req->copyHandle, (void*)TRANSFER_VA);
    if(size < 0)
    {
        svcCloseHandle(req->copyHandle);
        return GFX_ERR_INVALID;
    }

    transfer = (const uint8_t*)TRANSFER_VA;
    transferSize = size;
    return GFX_OK;
}

static long Gfx_CreateTexture(ServerRequest *req)
{
    const GfxCreateTextureIn *in = (const GfxCreateTextureIn*)req->in;
    if(in->width == 0 || in->height == 0 || in->width > MAX_TEXTURE_DIM || in->height > MAX_TEXTURE_DIM)
        return GFX_ERR_INVALID;

    for(uint32_t i = 0; i < MAX_TEXTURES; i++)
    {
        if(textures[i].pixels)
            continue;

        textures[i].pixels = (uint32_t*)calloc(in->width * in->height, sizeof(uint32_t));
        if(!textures[i].pixels)
            return GFX_ERR_NO_MEMORY;
        textures[i].width = in->width;
        textures[i].height = in->height;

        ((GfxTextureRef*)req->out)->texture = i + 1;
        req->outSize = sizeof(GfxTextureRef);
        return GFX_OK;
    }
    return GFX_ERR_NO_MEMORY;
}

static long Gfx_UpdateTexture(ServerRequest *req)
{
    const GfxUpdateTextureIn *in = (const GfxUpdateTextureIn*)req->in;
    RasterTexture *tex = GetTexture(in->texture);
    if(!tex || (uint64_t)in->x + in->width > tex->width || (uint64_t)in->y + in->height > tex->height)
        return GFX_ERR_INVALID;
    if(!TransferRange(in->offset, (uint64_t)in->width * in->height * sizeof(uint32_t)))
        return GFX_ERR_INVALID;

    const uint32_t *src = (const uint32_t*)(transfer + in->offset);
    for(uint32_t y = 0; y < in->height; y++)
        memcpy(&tex->pixels[(in->y + y) * tex->width + in->x], src + y * in->width, in->width * sizeof(uint32_t));
    return GFX_OK;
}

static long Gfx_DestroyTexture(ServerRequest *req)
{
    RasterTexture *tex = GetTexture(((const GfxTextureRef*)req->in)->texture);
    if(!tex)
        return GFX_ERR_INVALID;

    free(tex->pixels);
    tex->pixels = nullptr;
    return GFX_OK;
}

static long Gfx_Draw(ServerRequest *req)
{
    const GfxDrawIn *in = (const GfxDrawIn*)req->in;
    if(!TransferRange(in->vtxOffset, (uint64_t)in->vtxCount * sizeof(GfxVertex)) ||
        !TransferRange(in->idxOffset, (uint64_t)in->idxCount * sizeof(uint16_t)) ||
        !TransferRange(in->cmdOffset, (uint64_t)in->cmdCount * sizeof(GfxDrawCmd)))
        return GFX_ERR_INVALID;

    const GfxVertex *vertices = (const GfxVertex*)(transfer + in->vtxOffset);
    const uint16_t *indices = (const uint16_t*)(transfer + in->idxOffset);
    const GfxDrawCmd *cmds = (const GfxDrawCmd*)(transfer + in->cmdOffset);

    for(uint32_t i = 0; i < in->cmdCount; i++)
    {
        const GfxDrawCmd *cmd = &cmds[i];
        if((uint64_t)cmd->idxOffset + cmd->elemCount > in->idxCount)
            return GFX_ERR_INVALID;
        if(cmd->texture && !GetTexture(cmd->texture))
            return GFX_ERR_INVALID;
        Raster_DrawCmd(cmd, vertices, in->vtxCount, indices, GetTexture(cmd->texture));
    }
    return GFX_OK;
}

static size_t RequestSize(uint32_t cmd)
{
    switch(cmd)
    {
        case GFX_CMD_CREATE_TEXTURE: return sizeof(GfxCreateTextureIn);
        case GFX_CMD_UPDATE_TEXTURE: return sizeof(GfxUpdateTextureIn);
        case GFX_CMD_DESTROY_TEXTURE: return sizeof(GfxTextureRef);
        case GFX_CMD_BEGIN_FRAME: return sizeof(GfxBeginFrameIn);
        case GFX_CMD_DRAW: return sizeof(GfxDrawIn);
        default: return 0;
    }
}

static long Gfx_Handle(ServerRequest *req, void *ctx)
{
    UNUSED(ctx);
    if(req->inSize < RequestSize(req->cmd))
        return GFX_ERR_INVALID;

    switch(req->cmd)
    {
        case GFX_CMD_ATTACH:
            return Gfx_Attach(req);
        case GFX_CMD_CREATE_TEXTURE:
            return Gfx_CreateTexture(req);
        case GFX_CMD_UPDATE_TEXTURE:
            return Gfx_UpdateTexture(req);
        case GFX_CMD_DESTROY_TEXTURE:
            return Gfx_DestroyTexture(req);
        case GFX_CMD_BEGIN_FRAME:
            Raster_Clear(((const GfxBeginFrameIn*)req->in)->clearColor);
            return GFX_OK;
        case GFX_CMD_DRAW:
            return Gfx_Draw(req);
        case GFX_CMD_END_FRAME:
            Raster_Present((void*)USERLAND_FB_VIRT_BASE, MAIN_DISP_WIDTH * 4);
            return GFX_OK;
        default:
            return GFX_ERR_INVALID;
    }
}

int main() 
{
    if(Raster_Init(MAIN_DISP_WIDTH, MAIN_DISP_HEIGHT) != 0)
    {
        svcDebug("couldn't allocate render target");
        return 0;
    }

    long port = serverRegister(GFX_PORT_NAME);
    if(port < 0)
    {
        svcDebug("couldn't register port");
        return 0;
    }

    serverRun(port, Gfx_Handle, nullptr);
    svcDebug("server stopped");
    return 0;
}
