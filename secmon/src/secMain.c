#include "libraries/hardware/constants.h"
#include "libraries/hardware/mmio.h"
#include "libraries/hardware/mmio_vars.h"
#include "libraries/hardware/gic.h"
#include "libraries/hardware/uart.h"

int secMain(void *dtb_ptr)
{
    *(uint32_t*)ARM_LOCAL_ADDR = 0;
    *(uint32_t*)ARM_PRESCALER = 0x80000000;

    uint64_t mpidr = 0;
    asm volatile("mrs %0, MPIDR_EL1" : "=r"(mpidr));

    //Enable dist on all cores and cpu et al on core0
    GICD_Enable();
    if(!(mpidr & 0b11))
    {
        GICC_Enable();

        GIC_SetSpiDefaults();
        
        //Even tho PPI bypass GICD, we still need to reg it for GICC. (ARM moment)
        GICD_EnableIRQ(LOCAL_TIMER_IRQ_PNS);
        GICD_SetGroup(LOCAL_TIMER_IRQ_PNS, 1);
        GICD_SetPriority(LOCAL_TIMER_IRQ_PNS, GIC_PRI_HIGHEST_NONSECURE);

        //Init PL011
        UART0_Init();
    }
    
    return 0;
}