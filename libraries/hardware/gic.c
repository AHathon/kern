#include "libraries/hardware/gic.h"
#include "libraries/hardware/mmio_vars.h"
#include "libraries/hardware/mmio.h"
#include "libraries/hardware/debug.h"

char *GetGicVersion()
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

uint32_t GetGicMaxIRQs()
{
    uint32_t max_irqs = *(volatile uint32_t *)GICD_TYPER;
    max_irqs = 32 * ((max_irqs & 0x1F) + 1);
    return max_irqs;
}

void GicSetGroup(uint32_t irq, uint8_t grp)
{
    *(volatile uint32_t *)GICD_IGROUPR(irq / 32) |= (grp << (irq % 32));
}

void GicRouteIRQ(uint32_t irq)
{
    *(volatile uint32_t *)GICD_ICFGR(irq / 16) = (*(volatile uint32_t *)GICD_ICFGR(irq / 16) & ~(0b11 << ((irq % 16) * 2))) | (0b10 << ((irq % 16) * 2));

    //Set target CPU
    ((volatile uint8_t *)GICD_ITARGETSR(irq >> 2))[irq % 4] = GICD_ITARGETSR_CORE0;

    //Set priority
    ((volatile uint8_t *)GICD_IPRIORITYR(irq >> 2))[irq % 4] = 0;

    //Enable the IRQ bit
    volatile uint32_t *isenabler = (volatile uint32_t *)(GICD_ISENABLER(irq >> 5));
    *isenabler = (1 << (irq % 32));

    LOGT("Enabling GIC IRQ #%d\n", irq);
}

void GicEnable()
{
    uint32_t groupsEn = CTLR_EN_GRP0 | CTLR_EN_GRP1;

    //Enable GIC Distributor
    *(volatile uint32_t *)GICD_CTLR = groupsEn;

    //Enable GIC CPU Interface
    *(volatile uint32_t *)GICC_PMR = 0xFF;
    *(volatile uint32_t *)GICC_BPR = 0;
    *(volatile uint32_t *)GICC_CTLR = 0x1E7;

    LOGT("Initialized GIC [%s] (%d IRQs)\n", GetGicVersion(), GetGicMaxIRQs());
}

void GicDisable()
{
    *(volatile uint32_t *)GICD_CTLR = 0;
    *(volatile uint32_t *)GICC_CTLR = 0;
}