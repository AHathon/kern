#pragma once

#include "libraries/types.h"

#ifdef __cplusplus
extern "C" {
#endif

void Framebuffer_Init(uint32_t w, uint32_t h);
void Framebuffer_Attach(uint8_t *ptr, uint32_t w, uint32_t h, uint32_t p);
uint8_t *Framebuffer_GetPtr();
uint32_t Framebuffer_GetSize();
uint32_t Framebuffer_GetPitch();
void Framebuffer_DrawPixel(int32_t x, int32_t y, uint8_t attr);
void Framebuffer_DrawRect(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t attr, int32_t fill);

#ifdef __cplusplus
}
#endif