#include "rasterizer.h"
#include "libraries/libc/stdlib.h"

#define SUBPIXEL_BITS   8
#define SUBPIXEL_ONE    (1 << SUBPIXEL_BITS)
#define MAX_COORD       32767.0f
#define FIXED_BITS      16
#define FIXED_ONE       (1 << FIXED_BITS)
#define OPAQUE_WHITE    0xFFFFFFFF

typedef int64_t Fixed;

typedef struct
{
    Fixed r, g, b, a;
    Fixed u, v;
} Interp;

typedef struct
{
    const RasterTexture *tex;
    bool gradient;
    bool textured;
    uint32_t tint;
    int originX, originY;
    Interp origin, dx, dy;
} Shader;

typedef struct
{
    int32_t x, y;
} SnappedVertex;

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

static inline uint32_t Sample(const RasterTexture *tex, float u, float v)
{
    if(!tex)
        return OPAQUE_WHITE;

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

static inline uint32_t ToFramebuffer(uint32_t src)
{
    return ((src & 0xFF) << 16) | (src & 0xFF00) | ((src >> 16) & 0xFF);
}

//src is 0xAABBGGRR, dst is the framebuffer's 0x00RRGGBB
static inline void Blend(uint32_t *dst, uint32_t src)
{
    uint32_t a = src >> 24;
    if(a == 0)
        return;
    if(a == 255)
    {
        *dst = ToFramebuffer(src);
        return;
    }

    uint32_t sr = src & 0xFF;
    uint32_t sg = (src >> 8) & 0xFF;
    uint32_t sb = (src >> 16) & 0xFF;
    uint32_t d = *dst;
    uint32_t r = (sr * a + ((d >> 16) & 0xFF) * (255 - a) + 127) / 255;
    uint32_t g = (sg * a + ((d >> 8) & 0xFF) * (255 - a) + 127) / 255;
    uint32_t b = (sb * a + (d & 0xFF) * (255 - a) + 127) / 255;
    *dst = (r << 16) | (g << 8) | b;
}

static inline Fixed ToFixed(float v)
{
    return (Fixed)(v * FIXED_ONE);
}

static inline uint32_t Channel(Fixed v, int shift)
{
    v = (v + FIXED_ONE / 2) >> FIXED_BITS;
    if(v < 0) v = 0;
    if(v > 255) v = 255;
    return (uint32_t)v << shift;
}

static inline uint32_t PackColor(const Interp *at)
{
    return Channel(at->r, 0) | Channel(at->g, 8) | Channel(at->b, 16) | Channel(at->a, 24);
}

static inline uint32_t SampleFixed(const RasterTexture *tex, const Interp *at)
{
    Fixed x = at->u >> FIXED_BITS;
    Fixed y = at->v >> FIXED_BITS;
    if(x < 0) x = 0;
    if(y < 0) y = 0;
    if(x >= tex->width) x = tex->width - 1;
    if(y >= tex->height) y = tex->height - 1;
    return tex->pixels[y * tex->width + x];
}

static void FillFlat(uint32_t *dst, int count, uint32_t color)
{
    uint32_t alpha = color >> 24;
    if(alpha == 0)
        return;

    if(alpha == 255)
    {
        uint32_t pixel = ToFramebuffer(color);
        for(int i = 0; i < count; i++)
            dst[i] = pixel;
        return;
    }

    for(int i = 0; i < count; i++)
        Blend(&dst[i], color);
}

template<bool Gradient, bool Textured>
static void FillShaded(uint32_t *dst, int count, const Shader *shader, Interp at)
{
    for(int i = 0; i < count; i++)
    {
        uint32_t color = shader->tint;
        if(Gradient)
        {
            color = PackColor(&at);
            at.r += shader->dx.r;
            at.g += shader->dx.g;
            at.b += shader->dx.b;
            at.a += shader->dx.a;
        }

        uint32_t texel = shader->tint;
        if(Textured)
        {
            texel = SampleFixed(shader->tex, &at);
            at.u += shader->dx.u;
            at.v += shader->dx.v;
            if((texel >> 24) == 0)
                continue;
        }

        if(texel == OPAQUE_WHITE)
            Blend(&dst[i], color);
        else if(color == OPAQUE_WHITE)
            Blend(&dst[i], texel);
        else
            Blend(&dst[i], Modulate(texel, color));
    }
}

static void Plane(float a, float b, float c, float abX, float abY, float acX, float acY, float offsetX, float offsetY, Fixed *origin, Fixed *dx, Fixed *dy)
{
    float det = abX * acY - acX * abY;
    float ab = b - a;
    float ac = c - a;
    float gradX = (ab * acY - ac * abY) / det;
    float gradY = (ac * abX - ab * acX) / det;
    *origin = ToFixed(a + gradX * offsetX + gradY * offsetY);
    *dx = ToFixed(gradX);
    *dy = ToFixed(gradY);
}

static void Shader_Init(Shader *shader, const GfxVertex *a, const GfxVertex *b, const GfxVertex *c, const RasterTexture *tex, int originX, int originY)
{
    bool flatColor = a->color == b->color && b->color == c->color;
    bool flatUv = a->u == b->u && b->u == c->u && a->v == b->v && b->v == c->v;

    shader->tex = tex;
    shader->gradient = !flatColor;
    shader->textured = tex && !flatUv;
    shader->tint = flatUv ? Sample(tex, a->u, a->v) : a->color;
    if(flatColor && flatUv)
        shader->tint = Modulate(shader->tint, a->color);
    shader->originX = originX;
    shader->originY = originY;
    if(!shader->gradient && !shader->textured)
        return;

    float abX = b->x - a->x;
    float abY = b->y - a->y;
    float acX = c->x - a->x;
    float acY = c->y - a->y;
    float offsetX = originX + 0.5f - a->x;
    float offsetY = originY + 0.5f - a->y;

    if(shader->gradient)
    {
        Interp *o = &shader->origin;
        Interp *x = &shader->dx;
        Interp *y = &shader->dy;
        Plane(a->color & 0xFF, b->color & 0xFF, c->color & 0xFF, abX, abY, acX, acY, offsetX, offsetY, &o->r, &x->r, &y->r);
        Plane((a->color >> 8) & 0xFF, (b->color >> 8) & 0xFF, (c->color >> 8) & 0xFF, abX, abY, acX, acY, offsetX, offsetY, &o->g, &x->g, &y->g);
        Plane((a->color >> 16) & 0xFF, (b->color >> 16) & 0xFF, (c->color >> 16) & 0xFF, abX, abY, acX, acY, offsetX, offsetY, &o->b, &x->b, &y->b);
        Plane(a->color >> 24, b->color >> 24, c->color >> 24, abX, abY, acX, acY, offsetX, offsetY, &o->a, &x->a, &y->a);
    }

    if(shader->textured)
    {
        float width = tex->width;
        float height = tex->height;
        Plane(a->u * width, b->u * width, c->u * width, abX, abY, acX, acY, offsetX, offsetY, &shader->origin.u, &shader->dx.u, &shader->dy.u);
        Plane(a->v * height, b->v * height, c->v * height, abX, abY, acX, acY, offsetX, offsetY, &shader->origin.v, &shader->dx.v, &shader->dy.v);
    }
}

static inline Fixed Advance(Fixed origin, Fixed dx, Fixed dy, int x, int y)
{
    return origin + dx * x + dy * y;
}

static void Shader_Span(const Shader *shader, int y, int start, int end)
{
    uint32_t *dst = target + y * targetWidth + start;
    int count = end - start;
    if(!shader->gradient && !shader->textured)
    {
        FillFlat(dst, count, shader->tint);
        return;
    }

    int x = start - shader->originX;
    int row = y - shader->originY;
    const Interp *o = &shader->origin;
    const Interp *dx = &shader->dx;
    const Interp *dy = &shader->dy;

    Interp at;
    if(shader->gradient)
    {
        at.r = Advance(o->r, dx->r, dy->r, x, row);
        at.g = Advance(o->g, dx->g, dy->g, x, row);
        at.b = Advance(o->b, dx->b, dy->b, x, row);
        at.a = Advance(o->a, dx->a, dy->a, x, row);
    }
    if(shader->textured)
    {
        at.u = Advance(o->u, dx->u, dy->u, x, row);
        at.v = Advance(o->v, dx->v, dy->v, x, row);
    }

    if(shader->gradient && shader->textured)
        FillShaded<true, true>(dst, count, shader, at);
    else if(shader->gradient)
        FillShaded<true, false>(dst, count, shader, at);
    else
        FillShaded<false, true>(dst, count, shader, at);
}

static inline int32_t Snap(float v)
{
    if(!(v > -MAX_COORD)) v = -MAX_COORD;
    if(v > MAX_COORD) v = MAX_COORD;
    v *= SUBPIXEL_ONE;
    return (int32_t)(v < 0 ? v - 0.5f : v + 0.5f);
}

static inline int FloorPixel(int32_t v)
{
    return v >> SUBPIXEL_BITS;
}

static inline int CeilPixel(int32_t v)
{
    return (v + SUBPIXEL_ONE - 1) >> SUBPIXEL_BITS;
}

static inline int CenterPixel(int32_t v)
{
    return (v + SUBPIXEL_ONE / 2 - 1) >> SUBPIXEL_BITS;
}

static inline int Min3(int a, int b, int c)
{
    int m = a < b ? a : b;
    return m < c ? m : c;
}

static inline int Max3(int a, int b, int c)
{
    int m = a > b ? a : b;
    return m > c ? m : c;
}

static inline int64_t Edge(const SnappedVertex *a, const SnappedVertex *b, int64_t x, int64_t y)
{
    return (int64_t)(b->x - a->x) * (y - a->y) - (int64_t)(b->y - a->y) * (x - a->x);
}

static inline int64_t EdgeBias(const SnappedVertex *a, const SnappedVertex *b)
{
    int32_t dy = b->y - a->y;
    bool topLeft = dy < 0 || (dy == 0 && b->x > a->x);
    return topLeft ? 0 : 1;
}

static inline void ClipSpan(int64_t value, int64_t step, int x0, int64_t *start, int64_t *end)
{
    if(step > 0)
    {
        if(value < 0)
        {
            int64_t first = x0 + (-value + step - 1) / step;
            if(first > *start) *start = first;
        }
    }
    else if(value < 0)
    {
        *end = x0;
    }
    else if(step < 0)
    {
        int64_t last = x0 + value / -step + 1;
        if(last < *end) *end = last;
    }
}

static void DrawTriangle(const GfxVertex *a, const GfxVertex *b, const GfxVertex *c, const RasterTexture *tex, int clipX0, int clipY0, int clipX1, int clipY1)
{
    SnappedVertex sa = { Snap(a->x), Snap(a->y) };
    SnappedVertex sb = { Snap(b->x), Snap(b->y) };
    SnappedVertex sc = { Snap(c->x), Snap(c->y) };

    int64_t area = Edge(&sa, &sb, sc.x, sc.y);
    if(area == 0)
        return;
    if(area < 0)
    {
        SnappedVertex s = sb;
        sb = sc;
        sc = s;
    }

    int x0 = FloorPixel(Min3(sa.x, sb.x, sc.x));
    int y0 = FloorPixel(Min3(sa.y, sb.y, sc.y));
    int x1 = CeilPixel(Max3(sa.x, sb.x, sc.x));
    int y1 = CeilPixel(Max3(sa.y, sb.y, sc.y));
    if(x0 < clipX0) x0 = clipX0;
    if(y0 < clipY0) y0 = clipY0;
    if(x1 > clipX1) x1 = clipX1;
    if(y1 > clipY1) y1 = clipY1;
    if(x0 >= x1 || y0 >= y1)
        return;

    int64_t sampleX = (int64_t)x0 * SUBPIXEL_ONE + SUBPIXEL_ONE / 2;
    int64_t sampleY = (int64_t)y0 * SUBPIXEL_ONE + SUBPIXEL_ONE / 2;
    int64_t ea = Edge(&sb, &sc, sampleX, sampleY) - EdgeBias(&sb, &sc);
    int64_t eb = Edge(&sc, &sa, sampleX, sampleY) - EdgeBias(&sc, &sa);
    int64_t ec = Edge(&sa, &sb, sampleX, sampleY) - EdgeBias(&sa, &sb);

    int64_t stepAX = -(int64_t)(sc.y - sb.y) * SUBPIXEL_ONE;
    int64_t stepBX = -(int64_t)(sa.y - sc.y) * SUBPIXEL_ONE;
    int64_t stepCX = -(int64_t)(sb.y - sa.y) * SUBPIXEL_ONE;
    int64_t stepAY = (int64_t)(sc.x - sb.x) * SUBPIXEL_ONE;
    int64_t stepBY = (int64_t)(sa.x - sc.x) * SUBPIXEL_ONE;
    int64_t stepCY = (int64_t)(sb.x - sa.x) * SUBPIXEL_ONE;

    Shader shader;
    Shader_Init(&shader, a, b, c, tex, x0, y0);

    for(int y = y0; y < y1; y++, ea += stepAY, eb += stepBY, ec += stepCY)
    {
        int64_t start = x0;
        int64_t end = x1;
        ClipSpan(ea, stepAX, x0, &start, &end);
        ClipSpan(eb, stepBX, x0, &start, &end);
        ClipSpan(ec, stepCX, x0, &start, &end);
        if(start < end)
            Shader_Span(&shader, y, (int)start, (int)end);
    }
}

static bool IsRect(const GfxVertex *a, const GfxVertex *b, const GfxVertex *c, const GfxVertex *d, const RasterTexture *tex)
{
    if(a->y != b->y || b->x != c->x || c->y != d->y || d->x != a->x || a->x >= b->x || a->y >= c->y)
        return false;

    bool colorPlanar = (a->color == b->color && c->color == d->color) || (a->color == d->color && b->color == c->color);
    bool uvFlat = a->u == b->u && b->u == c->u && c->u == d->u && a->v == b->v && b->v == c->v && c->v == d->v;
    bool uvPlanar = tex && a->u == d->u && b->u == c->u && a->v == b->v && c->v == d->v;
    return colorPlanar && (uvFlat || uvPlanar);
}

static void DrawRect(const GfxVertex *a, const GfxVertex *b, const GfxVertex *c, const RasterTexture *tex, int clipX0, int clipY0, int clipX1, int clipY1)
{
    int x0 = CenterPixel(Snap(a->x));
    int y0 = CenterPixel(Snap(a->y));
    int x1 = CenterPixel(Snap(b->x));
    int y1 = CenterPixel(Snap(c->y));
    if(x0 < clipX0) x0 = clipX0;
    if(y0 < clipY0) y0 = clipY0;
    if(x1 > clipX1) x1 = clipX1;
    if(y1 > clipY1) y1 = clipY1;
    if(x0 >= x1 || y0 >= y1)
        return;

    Shader shader;
    Shader_Init(&shader, a, b, c, tex, x0, y0);
    for(int y = y0; y < y1; y++)
        Shader_Span(&shader, y, x0, x1);
}

void Raster_DrawCmd(const GfxDrawCmd *cmd, const GfxVertex *vertices, uint32_t vtxCount, const uint16_t *indices, const RasterTexture *tex)
{
    int clipX0 = cmd->clipX0 < 0 ? 0 : cmd->clipX0;
    int clipY0 = cmd->clipY0 < 0 ? 0 : cmd->clipY0;
    int clipX1 = cmd->clipX1 > targetWidth ? targetWidth : cmd->clipX1;
    int clipY1 = cmd->clipY1 > targetHeight ? targetHeight : cmd->clipY1;
    const uint16_t *idx = indices + cmd->idxOffset;

    for(uint32_t i = 0; i + 2 < cmd->elemCount; i += 3)
    {
        uint64_t ia = (uint64_t)cmd->vtxOffset + idx[i];
        uint64_t ib = (uint64_t)cmd->vtxOffset + idx[i + 1];
        uint64_t ic = (uint64_t)cmd->vtxOffset + idx[i + 2];
        if(ia >= vtxCount || ib >= vtxCount || ic >= vtxCount)
            continue;

        if(i + 5 < cmd->elemCount && idx[i + 3] == idx[i] && idx[i + 4] == idx[i + 2])
        {
            uint64_t id = (uint64_t)cmd->vtxOffset + idx[i + 5];
            if(id < vtxCount && IsRect(&vertices[ia], &vertices[ib], &vertices[ic], &vertices[id], tex))
            {
                DrawRect(&vertices[ia], &vertices[ib], &vertices[ic], tex, clipX0, clipY0, clipX1, clipY1);
                i += 3;
                continue;
            }
        }

        DrawTriangle(&vertices[ia], &vertices[ib], &vertices[ic], tex, clipX0, clipY0, clipX1, clipY1);
    }
}

void Raster_Present(void *framebuffer, int pitch)
{
    for(int y = 0; y < targetHeight; y++)
    {
        uint8_t *dst = (uint8_t*)framebuffer + y * pitch;
        const uint8_t *src = (const uint8_t*)(target + y * targetWidth);
        int bytes = targetWidth * sizeof(uint32_t);
        int i = 0;

        //The framebuffer is uncached, so wide writes are far cheaper than one per pixel
        for(; i + 64 <= bytes; i += 64)
        {
            asm volatile("ldp q0, q1, [%1]\n"
                         "ldp q2, q3, [%1, #32]\n"
                         "stp q0, q1, [%0]\n"
                         "stp q2, q3, [%0, #32]\n"
                         : : "r"(dst + i), "r"(src + i) : "v0", "v1", "v2", "v3", "memory");
        }
        for(; i < bytes; i += sizeof(uint32_t))
            *(volatile uint32_t*)(dst + i) = *(const uint32_t*)(src + i);
    }
}
