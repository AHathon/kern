#include "libraries/ui/terminal.h"
#include "libraries/hardware/framebuffer.h"
#include "libraries/hardware/debug.h"
#include "libraries/ui/font.h"

static int32_t termPosX = 10;
static int32_t termPosY = 10;

static int32_t termX = 0;
static int32_t termY = 0;

void Terminal_DrawChar(uint8_t ch, int32_t x, int32_t y, uint8_t attr)
{
    uint8_t *glyph = (uint8_t *)&font + (ch < FONT_NUMGLYPHS ? ch : 0) * FONT_BPG;

    for (int i=0;i<FONT_HEIGHT;i++) 
    {
        for (int j=0;j<FONT_WIDTH;j++) 
        {
            uint8_t mask = 1 << j;
            uint8_t col = (*glyph & mask) ? attr & 0x0f : (attr & 0xf0) >> 4;

            Framebuffer_DrawPixel(x+j, y+i, col);
        }
        glyph += FONT_BPL;
    }
}

void Terminal_DrawString(char *s, uint8_t attr)
{
    while (*s) 
    {
        if (*s == '\r')
            termX = termPosX;
        else if(*s == '\n')
        {
            termX = termPosX; 
            termY += (FONT_HEIGHT + (FONT_HEIGHT * 0.2));
        }
        else 
        {
            Terminal_DrawChar(*s, termX, termY, attr);
            termX += FONT_WIDTH;
        }
        s++;
    }
}

void Terminal_Init()
{
    Framebuffer_Init(MAIN_DISP_WIDTH, MAIN_DISP_HEIGHT);
    Terminal_Clear();
}

void Terminal_Write(const char *fmt, ...)
{
    char buf[256];
	va_list list;
	va_start(list, fmt);
	vasprintf(buf, fmt, list);
	Terminal_DrawString(buf, 0x0f);
	va_end(list);
}

void Terminal_Clear()
{
    termX = termPosX;
    termY = termPosY;
}