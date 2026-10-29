#include "libraries/hardware/framebuffer.h"
#include "libraries/hardware/constants.h"
#include "kernel/memory/kMemoryMap.h"
#include "libraries/types.h"

int main() 
{
    //init maps the framebuffer here before starting us
    Framebuffer_Attach((uint8_t*)USERLAND_FB_VIRT_BASE, MAIN_DISP_WIDTH, MAIN_DISP_HEIGHT, MAIN_DISP_WIDTH * 4);

    const int32_t barWidth = MAIN_DISP_WIDTH / 16;
    for(int32_t i = 0; i < 16; i++)
    {
        int32_t x = i * barWidth;
        Framebuffer_DrawRect(x, MAIN_DISP_HEIGHT - 80, x + barWidth - 1, MAIN_DISP_HEIGHT - 1, (i << 4) | 0xF, 1);
    }

    return 0;
}
