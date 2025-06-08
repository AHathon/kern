#pragma once

#include "libraries/types.h"

void Framebuffer_Init(uint32_t w, uint32_t h);
void Framebuffer_DrawPixel(int32_t x, int32_t y, uint8_t attr);
void Framebuffer_DrawRect(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t attr, int32_t fill);
void Framebuffer_DrawChar(uint8_t ch, int32_t x, int32_t y, uint8_t attr);
void Framebuffer_DrawString(int32_t x, int32_t y, char *s, uint8_t attr);