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
    //Only C1/C3 should be used for OS
    //GicRouteIRQ(SYSTEM_TIMER_IRQ_1);
    //GicRouteIRQ(SYSTEM_TIMER_IRQ_3);
    GicRouteIRQ(LOCAL_TIMER_IRQ_PNS);
    GicEnable();
    
    return 0;
}