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

    uint32_t max_ints = GIC_GetGicMaxIRQs();

    uint64_t mpidr = 0;
    asm volatile("mrs %0, MPIDR_EL1" : "=r"(mpidr));
    if(mpidr != 3) //primary core
    {
        GICD_Enable();
    }

    //Core0
    GICD_Enable();
    GICC_Enable();

    //Clear and disable all
    for(int i = 0; i < max_ints; i++)
    {
        GICD_ClearActiveIRQ(i);
        GICD_ClearPendingIRQ(i);
        GICD_DisableIRQ(i);
    }

    //Even tho PPI bypass GICD, we still need to reg it for GICC. (ARM moment)
    GICD_EnableIRQ(LOCAL_TIMER_IRQ_PNS);
    GICD_SetGroup(LOCAL_TIMER_IRQ_PNS, 1);
    
    return 0;
}