#include "libraries/hardware/gic.h"
#include "libraries/hardware/mmio_vars.h"
#include "libraries/hardware/mmio.h"
#include "libraries/hardware/debug.h"

char *GIC_GetGicVersion()
{
    uint32_t pidr = *(volatile uint32_t *)GICD_PIDR2V2;
    char *ver = 0;
    switch (pidr >> GIC_REV_OFFSET) {
        case 1:
            ver = "GICv1";
            break;
        case 2:
            ver = "GICv2";
            break;
        default:
            ver = "unknown";
    }
    return ver;
}

uint32_t GIC_GetGicMaxIRQs()
{
    uint32_t max_irqs = *(volatile uint32_t *)GICD_TYPER;
    max_irqs = 32 * ((max_irqs & 0x1F) + 1);
    return max_irqs;
}

void GICD_SetGroup(uint32_t irq, uint8_t grp)
{
    *(volatile uint32_t *)GICD_IGROUPR(irq / 32) |= (grp << (irq % 32));
}

void GICD_RouteIRQ(uint32_t irq)
{
    *(volatile uint32_t *)GICD_ICFGR(irq / 16) = 0;

    //Set target CPU
    ((volatile uint8_t *)GICD_ITARGETSR(irq >> 2))[irq % 4] = GICD_ITARGETSR_CORE0;

    //Set priority
    ((volatile uint8_t *)GICD_IPRIORITYR(irq >> 2))[irq % 4] = 0;

   
    //LOGT("Enabling GIC IRQ #%d\n", irq);
}

void GICD_ClearPendingIRQ(uint32_t irq)
{
    *(volatile uint32_t *)(GICD_ICPENDR(irq >> 5)) = (1 << (irq % 32));
}

void GICD_ClearActiveIRQ(uint32_t irq)
{
    *(volatile uint32_t *)(GICD_ICACTIVER(irq >> 5)) = (1 << (irq % 32));
}

void GICD_EnableIRQ(uint32_t irq)
{
    //Enable the IRQ bit in distributer
    *(volatile uint32_t *)(GICD_ISENABLER(irq >> 5)) = (1 << (irq % 32));
}

void GICD_DisableIRQ(uint32_t irq)
{
    *(volatile uint32_t *)(GICD_ICENABLER(irq >> 5)) = (1 << (irq % 32));
}

void GIC_Enable()
{
    uint32_t groupsEn = CTLR_EN_GRP0 | CTLR_EN_GRP1;

    //Enable GIC Distributor
    *(volatile uint32_t *)GICD_CTLR = groupsEn;

    //Enable GIC CPU Interface
    *(volatile uint32_t *)GICC_PMR = 0;
    *(volatile uint32_t *)GICC_BPR = 0;
    *(volatile uint32_t *)GICC_CTLR = 0x1E7;

    //LOGT("Initialized GIC [%s] (%d IRQs)\n", GIC_GetGicVersion(), GIC_GetGicMaxIRQs());
}

void GIC_Disable()
{
    *(volatile uint32_t *)GICD_CTLR = 0;
    *(volatile uint32_t *)GICC_CTLR = 0;
}