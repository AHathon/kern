#pragma once

#include "image.h"

class Wallpaper
{
public:
    Wallpaper() = default;
    ~Wallpaper() = default;

    void Finalize();
    void Draw();

    void SetColor(uint32_t rgb);
    bool SetImage(const uint32_t *pixels, uint32_t width, uint32_t height);
    bool SetBmp(const void *data, size_t size);
    void ClearImage();

private:
    Image image;
};
