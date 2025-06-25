#include "libraries/hardware/constants.h"
#include "libraries/hardware/mmio.h"
#include "libraries/hardware/mmio_vars.h"
#include "libraries/hardware/gic.h"
#include "libraries/hardware/uart.h"

int secMain(void *dtb_ptr)
{
    *(uint32_t*)ARM_LOCAL_ADDR = 0;
    *(uint32_t*)ARM_PRESCALER = 0x80000000;

    UART0_Init();

    GicDisable();
    //GicSetGroup(LOCAL_TIMER_IRQ_PNS, 1);
    //GicRouteIRQ(LOCAL_TIMER_IRQ_PNS);
    GicEnable();
    
    return 0;
}