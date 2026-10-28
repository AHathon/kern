#include "libraries/services/svc.h"
#include "libraries/hardware/framebuffer.h"
#include "libraries/hardware/mmio_vars.h"
#include "kernel/memory/kMemoryMap.h"
#include "libraries/types.h"

int main() 
{
    //KIPs run at EL1 but link their own copy of the libraries; point them at the kernel's linear map
    MMIO_ADDR = MMIO_BASE + KERNEL_VIRT_BASE;
    FB_OFF = KERNEL_VIRT_BASE;

    Framebuffer_Init(MAIN_DISP_WIDTH, MAIN_DISP_HEIGHT);

    uint64_t i = 0xFFFFFFF;
    while(i--) asm volatile("nop");
    return 0;
}
