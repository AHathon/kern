#pragma once

#include "libraries/types.h"

class Image
{
public:
    Image() = default;
    ~Image() = default;

    bool Load(const uint32_t *pixels, uint32_t newWidth, uint32_t newHeight);
    bool LoadBmp(const void *data, size_t size);
    bool LoadBmpFile(const char *path);
    void Unload();

    uint32_t Texture() const { return texture; }
    uint32_t Width() const { return width; }
    uint32_t Height() const { return height; }

private:
    uint32_t texture = 0;
    uint32_t width = 0;
    uint32_t height = 0;
};
