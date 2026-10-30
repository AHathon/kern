#include "rasterizer.h"
#include "libc/stdlib.h"
#include "libc/math.h"

static uint32_t *target = nullptr;
static int targetWidth = 0;
static int targetHeight = 0;

int Raster_Init(int width, int height)
{
    target = (uint32_t*)malloc(width * height * sizeof(uint32_t));
    targetWidth = width;
    targetHeight = height;
    return target ? 0 : -1;
}

void Raster_Clear(uint32_t color)
{
    int pixels = targetWidth * targetHeight;
    for(int i = 0; i < pixels; i++)
        target[i] = color;
}

static inline float Min3(float a, float b, float c)
{
    float m = a < b ? a : b;
    return m < c ? m : c;
}

static inline float Max3(float a, float b, float c)
{
    float m = a > b ? a : b;
    return m > c ? m : c;
}

static inline uint32_t Sample(const RasterTexture *tex, float u, float v)
{
    if(!tex)
        return 0xFFFFFFFF;

    int x = (int)(u * tex->width);
    int y = (int)(v * tex->height);
    if(x < 0) x = 0;
    if(y < 0) y = 0;
    if(x >= (int)tex->width) x = tex->width - 1;
    if(y >= (int)tex->height) y = tex->height - 1;
    return tex->pixels[y * tex->width + x];
}

static inline uint32_t Modulate(uint32_t a, uint32_t b)
{
    uint32_t out = 0;
    for(int shift = 0; shift < 32; shift += 8)
    {
        uint32_t ca = (a >> shift) & 0xFF;
        uint32_t cb = (b >> shift) & 0xFF;
        out |= ((ca * cb + 127) / 255) << shift;
    }
    return out;
}

//src is 0xAABBGGRR, dst is the framebuffer's 0x00RRGGBB
static inline void Blend(uint32_t *dst, uint32_t src)
{
    uint32_t a = src >> 24;
    if(a == 0)
        return;

    uint32_t sr = src & 0xFF;
    uint32_t sg = (src >> 8) & 0xFF;
    uint32_t sb = (src >> 16) & 0xFF;
    if(a == 255)
    {
        *dst = (sr << 16) | (sg << 8) | sb;
        return;
    }

    uint32_t d = *dst;
    uint32_t r = (sr * a + ((d >> 16) & 0xFF) * (255 - a) + 127) / 255;
    uint32_t g = (sg * a + ((d >> 8) & 0xFF) * (255 - a) + 127) / 255;
    uint32_t b = (sb * a + (d & 0xFF) * (255 - a) + 127) / 255;
    *dst = (r << 16) | (g << 8) | b;
}

static inline uint32_t LerpColor(uint32_t a, uint32_t b, uint32_t c, float wa, float wb, float wc)
{
    uint32_t out = 0;
    for(int shift = 0; shift < 32; shift += 8)
    {
        float v = ((a >> shift) & 0xFF) * wa + ((b >> shift) & 0xFF) * wb + ((c >> shift) & 0xFF) * wc;
        out |= (uint32_t)(v + 0.5f) << shift;
    }
    return out;
}

static inline float Edge(const GfxVertex *a, const GfxVertex *b, float x, float y)
{
    return (b->x - a->x) * (y - a->y) - (b->y - a->y) * (x - a->x);
}

static inline float EdgeBias(const GfxVertex *a, const GfxVertex *b)
{
    float dy = b->y - a->y;
    bool topLeft = dy < 0 || (dy == 0 && b->x > a->x);
    return topLeft ? 0.0f : 1e-4f;
}

static void DrawTriangle(const GfxVertex *a, const GfxVertex *b, const GfxVertex *c, const RasterTexture *tex, int clipX0, int clipY0, int clipX1, int clipY1)
{
    float area = Edge(a, b, c->x, c->y);
    if(area == 0)
        return;
    if(area < 0)
    {
        const GfxVertex *t = b;
        b = c;
        c = t;
        area = -area;
    }

    int x0 = (int)floorf(Min3(a->x, b->x, c->x));
    int y0 = (int)floorf(Min3(a->y, b->y, c->y));
    int x1 = (int)ceilf(Max3(a->x, b->x, c->x));
    int y1 = (int)ceilf(Max3(a->y, b->y, c->y));
    if(x0 < clipX0) x0 = clipX0;
    if(y0 < clipY0) y0 = clipY0;
    if(x1 > clipX1) x1 = clipX1;
    if(y1 > clipY1) y1 = clipY1;
    if(x0 >= x1 || y0 >= y1)
        return;

    float invArea = 1.0f / area;
    float biasA = EdgeBias(b, c);
    float biasB = EdgeBias(c, a);
    float biasC = EdgeBias(a, b);

    float stepAX = -(c->y - b->y);
    float stepBX = -(a->y - c->y);
    float stepCX = -(b->y - a->y);

    bool flatColor = a->color == b->color && b->color == c->color;
    bool flatUv = a->u == b->u && b->u == c->u && a->v == b->v && b->v == c->v;
    uint32_t flat = a->color;
    if(flatUv)
        flat = Modulate(Sample(tex, a->u, a->v), a->color);

    for(int y = y0; y < y1; y++)
    {
        float px = x0 + 0.5f;
        float py = y + 0.5f;
        float ea = Edge(b, c, px, py);
        float eb = Edge(c, a, px, py);
        float ec = Edge(a, b, px, py);

        uint32_t *row = target + y * targetWidth;
        for(int x = x0; x < x1; x++, ea += stepAX, eb += stepBX, ec += stepCX)
        {
            if(ea < biasA || eb < biasB || ec < biasC)
                continue;

            if(flatColor && flatUv)
            {
                Blend(&row[x], flat);
                continue;
            }

            float wa = ea * invArea;
            float wb = eb * invArea;
            float wc = ec * invArea;
            uint32_t color = flatColor ? a->color : LerpColor(a->color, b->color, c->color, wa, wb, wc);
            if(flatUv)
                color = Modulate(Sample(tex, a->u, a->v), color);
            else
                color = Modulate(Sample(tex, a->u * wa + b->u * wb + c->u * wc, a->v * wa + b->v * wb + c->v * wc), color);
            Blend(&row[x], color);
        }
    }
}

void Raster_DrawCmd(const GfxDrawCmd *cmd, const GfxVertex *vertices, uint32_t vtxCount, const uint16_t *indices, const RasterTexture *tex)
{
    int clipX0 = cmd->clipX0 < 0 ? 0 : cmd->clipX0;
    int clipY0 = cmd->clipY0 < 0 ? 0 : cmd->clipY0;
    int clipX1 = cmd->clipX1 > targetWidth ? targetWidth : cmd->clipX1;
    int clipY1 = cmd->clipY1 > targetHeight ? targetHeight : cmd->clipY1;

    for(uint32_t i = 0; i + 2 < cmd->elemCount; i += 3)
    {
        uint64_t ia = (uint64_t)cmd->vtxOffset + indices[cmd->idxOffset + i];
        uint64_t ib = (uint64_t)cmd->vtxOffset + indices[cmd->idxOffset + i + 1];
        uint64_t ic = (uint64_t)cmd->vtxOffset + indices[cmd->idxOffset + i + 2];
        if(ia >= vtxCount || ib >= vtxCount || ic >= vtxCount)
            continue;
        DrawTriangle(&vertices[ia], &vertices[ib], &vertices[ic], tex, clipX0, clipY0, clipX1, clipY1);
    }
}

void Raster_Present(void *framebuffer, int pitch)
{
    for(int y = 0; y < targetHeight; y++)
    {
        volatile uint32_t *dst = (volatile uint32_t*)((uint8_t*)framebuffer + y * pitch);
        const uint32_t *src = target + y * targetWidth;
        for(int x = 0; x < targetWidth; x++)
            dst[x] = src[x];
    }
}
