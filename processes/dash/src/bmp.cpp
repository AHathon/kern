#include "bmp.h"

#define FILE_HEADER_SIZE    14
#define INFO_HEADER_SIZE    40
#define MASKS_OFFSET        (FILE_HEADER_SIZE + INFO_HEADER_SIZE)
#define V3_HEADER_SIZE      56
#define MAX_DIMENSION       4096

#define COMPRESSION_NONE            0
#define COMPRESSION_BITFIELDS       3
#define COMPRESSION_ALPHABITFIELDS  6

static uint16_t Read16(const uint8_t *p)
{
    return p[0] | (p[1] << 8);
}

static uint32_t Read32(const uint8_t *p)
{
    return p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24);
}

static int MaskShift(uint32_t mask)
{
    for(int shift = 0; shift < 32; shift += 8)
        if(mask == 0xFFu << shift)
            return shift;
    return -1;
}

static bool ReadMasks(const uint8_t *bytes, size_t size, uint32_t headerSize, uint32_t compression, BmpInfo *info)
{
    bool hasAlpha = compression == COMPRESSION_ALPHABITFIELDS || headerSize >= V3_HEADER_SIZE;
    if(size < MASKS_OFFSET + (hasAlpha ? 16u : 12u))
        return false;

    info->redShift = MaskShift(Read32(bytes + MASKS_OFFSET));
    info->greenShift = MaskShift(Read32(bytes + MASKS_OFFSET + 4));
    info->blueShift = MaskShift(Read32(bytes + MASKS_OFFSET + 8));
    if(info->redShift < 0 || info->greenShift < 0 || info->blueShift < 0)
        return false;

    uint32_t alphaMask = hasAlpha ? Read32(bytes + MASKS_OFFSET + 12) : 0;
    info->alphaShift = alphaMask ? MaskShift(alphaMask) : -1;
    return !alphaMask || info->alphaShift >= 0;
}

bool Bmp_ReadInfo(const void *data, size_t size, BmpInfo *info)
{
    const uint8_t *bytes = (const uint8_t*)data;
    if(size < MASKS_OFFSET || bytes[0] != 'B' || bytes[1] != 'M')
        return false;

    uint32_t headerSize = Read32(bytes + 14);
    int32_t width = (int32_t)Read32(bytes + 18);
    int64_t height = (int32_t)Read32(bytes + 22);
    uint32_t bitsPerPixel = Read16(bytes + 28);
    uint32_t compression = Read32(bytes + 30);

    info->topDown = height < 0;
    if(info->topDown)
        height = -height;

    if(headerSize < INFO_HEADER_SIZE || Read16(bytes + 26) != 1)
        return false;
    if(width <= 0 || height <= 0 || width > MAX_DIMENSION || height > MAX_DIMENSION)
        return false;
    if(bitsPerPixel != 24 && bitsPerPixel != 32)
        return false;

    if(compression == COMPRESSION_NONE)
    {
        info->redShift = 16;
        info->greenShift = 8;
        info->blueShift = 0;
        info->alphaShift = bitsPerPixel == 32 ? 24 : -1;
    }
    else if(bitsPerPixel != 32 || (compression != COMPRESSION_BITFIELDS && compression != COMPRESSION_ALPHABITFIELDS) ||
        !ReadMasks(bytes, size, headerSize, compression, info))
        return false;

    info->width = width;
    info->height = height;
    info->pixelOffset = Read32(bytes + 10);
    info->bytesPerPixel = bitsPerPixel / 8;
    info->stride = ((width * bitsPerPixel + 31) / 32) * 4;

    return info->pixelOffset >= MASKS_OFFSET && (uint64_t)info->pixelOffset + (uint64_t)info->stride * info->height <= size;
}

void Bmp_Decode(const void *data, const BmpInfo *info, uint32_t *pixels)
{
    const uint8_t *bytes = (const uint8_t*)data + info->pixelOffset;
    bool anyAlpha = false;

    for(uint32_t y = 0; y < info->height; y++)
    {
        const uint8_t *src = bytes + (info->topDown ? y : info->height - 1 - y) * info->stride;
        uint32_t *dst = pixels + y * info->width;

        for(uint32_t x = 0; x < info->width; x++, src += info->bytesPerPixel)
        {
            uint32_t value = src[0] | (src[1] << 8) | (src[2] << 16);
            if(info->bytesPerPixel == 4)
                value |= (uint32_t)src[3] << 24;

            uint32_t alpha = info->alphaShift < 0 ? 0xFF : (value >> info->alphaShift) & 0xFF;
            anyAlpha |= alpha != 0;
            dst[x] = ((value >> info->redShift) & 0xFF) | (((value >> info->greenShift) & 0xFF) << 8) |
                (((value >> info->blueShift) & 0xFF) << 16) | (alpha << 24);
        }
    }

    if(!anyAlpha)
        for(uint32_t i = 0; i < info->width * info->height; i++)
            pixels[i] |= 0xFF000000;
}
