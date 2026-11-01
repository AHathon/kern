#include "image.h"
#include "bmp.h"
#include "services.h"
#include "libraries/libc/stdlib.h"

bool Image::Load(const uint32_t *pixels, uint32_t newWidth, uint32_t newHeight)
{
    uint32_t loaded = 0;
    if(gfxCreateTexture(&gfx, newWidth, newHeight, &loaded) != GFX_OK)
        return false;

    if(gfxUpdateTexture(&gfx, loaded, 0, 0, newWidth, newHeight, pixels, newWidth * sizeof(uint32_t)) != GFX_OK)
    {
        gfxDestroyTexture(&gfx, loaded);
        return false;
    }

    Unload();
    texture = loaded;
    width = newWidth;
    height = newHeight;
    return true;
}

bool Image::LoadBmp(const void *data, size_t size)
{
    BmpInfo info;
    if(!Bmp_ReadInfo(data, size, &info))
        return false;

    uint32_t *pixels = (uint32_t*)malloc(info.width * info.height * sizeof(uint32_t));
    if(!pixels)
        return false;

    Bmp_Decode(data, &info, pixels);
    bool loaded = Load(pixels, info.width, info.height);
    free(pixels);
    return loaded;
}

bool Image::LoadBmpFile(const char *path)
{
    uint8_t probe;
    uint32_t got = 0;
    uint64_t size = 0;
    if(fsReadFile(&fs, path, 0, &probe, sizeof(probe), &got, &size) != FS_OK)
        return false;

    uint8_t *data = (uint8_t*)malloc(size);
    if(!data)
        return false;

    uint64_t off = 0;
    while(off < size)
    {
        uint64_t chunk = size - off;
        if(chunk > IPC_MAX_COPY)
            chunk = IPC_MAX_COPY;
        if(fsReadFile(&fs, path, off, data + off, chunk, &got, 0) != FS_OK || got == 0)
            break;
        off += got;
    }

    bool loaded = off == size && LoadBmp(data, size);
    free(data);
    return loaded;
}

void Image::Unload()
{
    if(texture)
        gfxDestroyTexture(&gfx, texture);
    texture = 0;
    width = 0;
    height = 0;
}
