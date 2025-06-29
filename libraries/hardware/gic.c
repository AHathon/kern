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

void GIC_SetSpiDefaults()
{
    uint32_t max_irqs = GIC_GetGicMaxIRQs();

    for(int i = MIN_SPI_ID; i < max_irqs; i+=32)
        *(volatile uint32_t *)GICD_IGROUPR(i >> 5) = ~0U;

    for(int i = MIN_SPI_ID; i < max_irqs; i+=4)
        *(volatile uint32_t *)GICD_IPRIORITYR(i >> 2) = ~0U;

    for(int i = MIN_SPI_ID; i < max_irqs; i+=16)
        *(volatile uint32_t *)GICD_ICFGR(i >> 4) = 0;
}

void GICD_SetGroup(uint32_t irq, uint8_t grp)
{
    if (grp == 1) {
        *(volatile uint32_t *)GICD_IGROUPR(irq >> 5) |= (1 << (irq % 32));
    } else {
        *(volatile uint32_t *)GICD_IGROUPR(irq >> 5) &= ~(1 << (irq % 32));
    }
}

void GICD_SetTargetCPU(uint32_t irq, uint32_t target)
{
    //For PPIs and SGIs, the target is fixed by hardware.
    if (irq < 32) return;
    ((volatile uint8_t *)GICD_ITARGETSR(irq >> 2))[irq % 4] |= (uint8_t)target;
}

void GICD_SetPriority(uint32_t irq, uint8_t priority)
{
    ((volatile uint8_t *)GICD_IPRIORITYR(irq >> 2))[irq % 4] = priority;
}

void GICD_SetTriggerType(uint32_t irq, uint8_t type)
{
    volatile uint32_t *reg_addr = (volatile uint32_t *)GICD_ICFGR(irq / 16);
    uint32_t currval = *reg_addr;
    
    currval &= ~(0b11 << (irq % 16) * 2);
    currval |= ((type & 0b11) << ((irq % 16) * 2));
    
    *reg_addr = currval;
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
    *(volatile uint32_t *)(GICD_ISENABLER(irq >> 5)) |= (1 << (irq % 32));
}

uint8_t GICD_IsIRQEnabled(uint32_t irq)
{
    return (*(volatile uint32_t *)(GICD_ISENABLER(irq >> 5)) >> (irq % 32)) & 1;
}

void GICD_DisableIRQ(uint32_t irq)
{
    *(volatile uint32_t *)(GICD_ICENABLER(irq >> 5)) = (1 << (irq % 32));
}

void GICC_Enable()
{
    //Enable GIC CPU Interface
    *(volatile uint32_t *)GICC_PMR = GIC_PRI_HIGHEST_NONSECURE;
    *(volatile uint32_t *)GICC_BPR = 0;
    *(volatile uint32_t *)GICC_CTLR = CTLR_EN_GRP1;
}

void GICC_Disable()
{
    *(volatile uint32_t *)GICC_CTLR = 0;
}

void GICD_Enable()
{
    //Enable GIC Distributor
    *(volatile uint32_t *)GICD_CTLR = CTLR_EN_GRP1;
}

void GICD_Disable()
{
    *(volatile uint32_t *)GICD_CTLR = 0;
}