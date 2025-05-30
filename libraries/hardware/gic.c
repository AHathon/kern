#include "libraries/hardware/gic.h"
#include "libraries/hardware/mmio_vars.h"
#include "libraries/hardware/mmio.h"

void setup_gic(uint32_t irq)
{
    uint32_t irqn = irq % 0x20;
    
    //Enable GIC Distributer
    *(volatile uint8_t *)GICD_CTLR = 1;
    //Enable GIC CPU Interface
    *(volatile uint8_t *)GICC_CTLR = 1;
    //Set target cpu for irq
    *(volatile uint8_t *)(GICD_ITARGETSR(irqn)) = 1;
    //Enable interrupt
    *(volatile uint32_t *)(GICD_ISENABLER(irq >> 5)) = (1 << irqn);
}