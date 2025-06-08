#include "libraries/hardware/framebuffer.h"
#include "libraries/hardware/mailbox.h"
#include "libraries/hardware/debug.h"

unsigned int width, height, pitch, isrgb;
uint8_t *fb;

unsigned int vgapal[] = {
    0x000000,
    0x0000AA,
    0x00AA00,
    0x00AAAA,
    0xAA0000,
    0xAA00AA,
    0xAA5500,
    0xAAAAAA,
    0x555555,
    0x5555FF,
    0x55FF55,
    0x55FFFF,
    0xFF5555,
    0xFF55FF,
    0xFFFF55,
    0xFFFFFF
};

void Framebuffer_Init(uint32_t w, uint32_t h)
{
    mbox[0] = 35*4; // Length of message in bytes
    mbox[1] = MBOX_REQUEST;

    mbox[2] = TAG_SETPHYWH; // Tag identifier
    mbox[3] = 8; // Value size in bytes
    mbox[4] = 0;
    mbox[5] = w; // Value(width)
    mbox[6] = h; // Value(height)

    mbox[7] = TAG_SETVIRTWH;
    mbox[8] = 8;
    mbox[9] = 8;
    mbox[10] = w;
    mbox[11] = h;

    mbox[12] = TAG_SETVIRTOFF;
    mbox[13] = 8;
    mbox[14] = 8;
    mbox[15] = 0; // Value(x)
    mbox[16] = 0; // Value(y)

    mbox[17] = TAG_SETDEPTH;
    mbox[18] = 4;
    mbox[19] = 4;
    mbox[20] = 32; // Bits per pixel

    mbox[21] = TAG_SETPXLORDR;
    mbox[22] = 4;
    mbox[23] = 4;
    mbox[24] = 1; // RGB

    mbox[25] = TAG_GETFB;
    mbox[26] = 8;
    mbox[27] = 8;
    mbox[28] = 4096; // FrameBufferInfo.pointer
    mbox[29] = 0;    // FrameBufferInfo.size

    mbox[30] = TAG_GETPITCH;
    mbox[31] = 4;
    mbox[32] = 4;
    mbox[33] = 0; // Bytes per line

    mbox[34] = TAG_LAST;

    // Check call is successful and we have a pointer with depth 32
    if (Mailbox_Call(CH_PROP) && mbox[20] == 32 && mbox[28] != 0) 
    {
        mbox[28] &= 0x3FFFFFFF; // Convert GPU address to ARM address
        fb = (uint8_t *)(KERNEL_VIRT_BASE + mbox[28]);
    }
}

void Framebuffer_DrawPixel(int32_t x, int32_t y, uint8_t attr)
{
    int32_t offs = (y * mbox[33]) + (x * 4);
    *((uint32_t*)(fb + offs)) = vgapal[attr & 0x0f];
}

void Framebuffer_DrawRect(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t attr, int32_t fill)
{
    int y=y1;

    while (y <= y2) {
        int x=x1;
        while (x <= x2) {
            if ((x == x1 || x == x2) || (y == y1 || y == y2)) 
                Framebuffer_DrawPixel(x, y, attr);
            else if (fill) 
                Framebuffer_DrawPixel(x, y, (attr & 0xf0) >> 4);
            x++;
        }
        y++;
    }
}