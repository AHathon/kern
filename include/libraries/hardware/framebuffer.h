#pragma once

#include "libraries/types.h"

#ifdef __cplusplus
extern "C" {
#endif

void Framebuffer_Init(uint32_t w, uint32_t h);
uint8_t *Framebuffer_GetPtr();
void Framebuffer_DrawPixel(int32_t x, int32_t y, uint8_t attr);
void Framebuffer_DrawRect(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t attr, int32_t fill);

#ifdef __cplusplus
}
#endif