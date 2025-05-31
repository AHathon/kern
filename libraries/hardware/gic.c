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

void GicSetup(uint32_t irq)
{
    //Set target CPU
    ((volatile uint8_t *)GICD_ITARGETSR(irq / 4))[irq % 4] = GICD_ITARGETSR_CORE0;

    //Set priority
    ((volatile uint8_t *)GICD_IPRIORITYR(irq / 4))[irq % 4] = GIC_PRI_HIGHEST_NONSECURE;

    // Enable the IRQ bit
    volatile uint32_t *isenabler = (volatile uint32_t *)(GICD_ISENABLER(irq >> 5));
    *isenabler |= (1 << (irq % 32));

    kprintf("Enabling GIC IRQ #%d\n", irq);
}

void GicInit()
{
    // Enable GIC Distributor (do this once globally, better outside)
    *(volatile uint32_t *)GICD_CTLR = 1;

    // Enable GIC CPU Interface (do this once globally)
    *(volatile uint32_t *)GICC_PMR = 0xFF;
    *(volatile uint32_t *)GICC_CTLR = 1;

    kprintf("Initialized GIC [%s] (%d IRQs)\n", GetGicVersion(), GetGicMaxIRQs());
}