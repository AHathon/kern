#pragma once

#include "libraries/hardware/constants.h"

//Legacy interrupt controller
#define IRQ_BASIC_PENDING	(MMIO_ADDR+0xB200)
#define IRQ_PENDING_1		(MMIO_ADDR+0xB204)
#define IRQ_PENDING_2		(MMIO_ADDR+0xB208)
#define FIQ_CONTROL		    (MMIO_ADDR+0xB20C)
#define ENABLE_IRQS_1		(MMIO_ADDR+0xB210)
#define ENABLE_IRQS_2		(MMIO_ADDR+0xB214)
#define ENABLE_BASIC_IRQS	(MMIO_ADDR+0xB218)
#define DISABLE_IRQS_1		(MMIO_ADDR+0xB21C)
#define DISABLE_IRQS_2		(MMIO_ADDR+0xB220)
#define DISABLE_BASIC_IRQS	(MMIO_ADDR+0xB224)

#define ARM_PRESCALER       (ARM_LOCAL_ADDR+0x08)

//Video core mailbox
#define VC_MBOX_BASE        (MMIO_ADDR+0xB880)

#define VC_MBOX_READ        ((volatile uint32_t*)(VC_MBOX_BASE+0x0000))
#define VC_MBOX_POLL        ((volatile uint32_t*)(VC_MBOX_BASE+0x0010))
#define VC_MBOX_SENDER      ((volatile uint32_t*)(VC_MBOX_BASE+0x0014))
#define VC_MBOX_STATUS      ((volatile uint32_t*)(VC_MBOX_BASE+0x0018))
#define VC_MBOX_CONFIG      ((volatile uint32_t*)(VC_MBOX_BASE+0x001C))
#define VC_MBOX_WRITE       ((volatile uint32_t*)(VC_MBOX_BASE+0x0020))

#define MBOX_TAG_SETPOWER    0x28001,
#define MBOX_TAG_SETCLKRATE  0x38002,

#define MBOX_RESPONSE       0x80000000
#define MBOX_FULL           0x80000000
#define MBOX_EMPTY          0x40000000

//GPIO
#define GPFSEL(n)       ((volatile uint32_t*)(GPIO_BASE+0x0000 + ((n) * 4))) //n = 0-5
#define GPSET(n)        ((volatile uint32_t*)(GPIO_BASE+0x001C + ((n) * 4))) //n = 0-1
#define GPCLR(n)        ((volatile uint32_t*)(GPIO_BASE+0x0028 + ((n) * 4))) //n = 0-1
#define GPLEV(n)        ((volatile uint32_t*)(GPIO_BASE+0x0034 + ((n) * 4))) //n = 0-1
#define GPEDS(n)        ((volatile uint32_t*)(GPIO_BASE+0x0040 + ((n) * 4))) //n = 0-1
#define GPHEN(n)        ((volatile uint32_t*)(GPIO_BASE+0x0064 + ((n) * 4))) //n = 0-1
#define GPPUD           ((volatile uint32_t*)(GPIO_BASE+0x0094))
#define GPPUDCLK0       ((volatile uint32_t*)(GPIO_BASE+0x0098))
#define GPPUDCLK1       ((volatile uint32_t*)(GPIO_BASE+0x009C))

#define FUNC_IN  0
#define FUNC_OUT 1
#define FUNC_A0  4
#define FUNC_A1  5
#define FUNC_A2  6
#define FUNC_A3  7
#define FUNC_A4  3
#define FUNC_A5  2

//UART0
#define UART0_DR     ((volatile uint32_t*)(UART0_BASE+0x0000))
#define UART0_FR     ((volatile uint32_t*)(UART0_BASE+0x0018))
#define UART0_IBRD   ((volatile uint32_t*)(UART0_BASE+0x0024))
#define UART0_FBRD   ((volatile uint32_t*)(UART0_BASE+0x0028))
#define UART0_LCRH   ((volatile uint32_t*)(UART0_BASE+0x002C))
#define UART0_CR     ((volatile uint32_t*)(UART0_BASE+0x0030))
#define UART0_IMSC   ((volatile uint32_t*)(UART0_BASE+0x0038))
#define UART0_ICR    ((volatile uint32_t*)(UART0_BASE+0x0044))

#define UART_FR_TXFF (1 << 5)
#define UART_FR_RXFE (1 << 4)

//UART1
#define UART1_ENABLE    ((volatile uint32_t *)(UART1_BASE+0x5004))
#define UART1_IO        ((volatile uint32_t *)(UART1_BASE+0x5040))
#define UART1_IER       ((volatile uint32_t *)(UART1_BASE+0x5044))
#define UART1_IIR       ((volatile uint32_t *)(UART1_BASE+0x5048))
#define UART1_LCR       ((volatile uint32_t *)(UART1_BASE+0x504C))
#define UART1_MCR       ((volatile uint32_t *)(UART1_BASE+0x5050))
#define UART1_LSR       ((volatile uint32_t *)(UART1_BASE+0x5054))
#define UART1_MSR       ((volatile uint32_t *)(UART1_BASE+0x5058))
#define UART1_SCRATCH   ((volatile uint32_t *)(UART1_BASE+0x505C))
#define UART1_CNTL      ((volatile uint32_t *)(UART1_BASE+0x5060))
#define UART1_STAT      ((volatile uint32_t *)(UART1_BASE+0x5064))
#define UART1_BAUD      ((volatile uint32_t *)(UART1_BASE+0x5068))

//Local interrupts
#define CONTROL_REGISTER                (ARM_LOCAL_ADDR+0x0000)
#define CORE0_TIMER_INTERRUPT_CTRL      (ARM_LOCAL_ADDR+0x0040)
#define CORE1_TIMER_INTERRUPT_CTRL      (ARM_LOCAL_ADDR+0x0044)
#define CORE2_TIMER_INTERRUPT_CTRL      (ARM_LOCAL_ADDR+0x0048)
#define CORE3_TIMER_INTERRUPT_CTRL      (ARM_LOCAL_ADDR+0x004C)
#define CORE0_MAILBOX_INTERRUPT_CTRL    (ARM_LOCAL_ADDR+0x0050)
#define CORE1_MAILBOX_INTERRUPT_CTRL    (ARM_LOCAL_ADDR+0x0054)
#define CORE2_MAILBOX_INTERRUPT_CTRL    (ARM_LOCAL_ADDR+0x0058)
#define CORE3_MAILBOX_INTERRUPT_CTRL    (ARM_LOCAL_ADDR+0x005C)
#define CORE0_IRQ_SOURCE                (ARM_LOCAL_ADDR+0x0060)
#define CORE1_IRQ_SOURCE                (ARM_LOCAL_ADDR+0x0064)
#define CORE2_IRQ_SOURCE                (ARM_LOCAL_ADDR+0x0068)
#define CORE3_IRQ_SOURCE                (ARM_LOCAL_ADDR+0x006C)
#define CORE0_FIQ_SOURCE                (ARM_LOCAL_ADDR+0x0070)
#define CORE1_FIQ_SOURCE                (ARM_LOCAL_ADDR+0x0074)
#define CORE2_FIQ_SOURCE                (ARM_LOCAL_ADDR+0x0078)
#define CORE3_FIQ_SOURCE                (ARM_LOCAL_ADDR+0x007C)

//BCM timer
#define SYS_TIMER_BASE  (MMIO_ADDR + 0x3000)
#define SYS_TIMER_CS    (SYS_TIMER_BASE + 0x0000)
#define SYS_TIMER_CS_M0 (1 << 0)
#define SYS_TIMER_CS_M1 (1 << 1)
#define SYS_TIMER_CS_M2 (1 << 2)
#define SYS_TIMER_CS_M3 (1 << 3)
#define SYS_TIMER_CLO   (SYS_TIMER_BASE+0x0004)
#define SYS_TIMER_CHI   (SYS_TIMER_BASE+0x0008)
#define SYS_TIMER_C0    (SYS_TIMER_BASE+0x000C)
#define SYS_TIMER_C1    (SYS_TIMER_BASE+0x0010)
#define SYS_TIMER_C2    (SYS_TIMER_BASE+0x0014)
#define SYS_TIMER_C3    (SYS_TIMER_BASE+0x0018)

//GIC Distributer
#define GICD_DIST_BASE (GICC_ADDR+0x00001000)

#define GICD_CTLR               (GICD_DIST_BASE+0x0000)
#define GICD_TYPER              (GICD_DIST_BASE+0x0004)
#define GICD_IGROUPR(n)         (GICD_DIST_BASE+0x0080 + (n * 4))
#define GICD_ISENABLER(n)       (GICD_DIST_BASE+0x0100 + (n * 4))
#define GICD_ICENABLER(n)       (GICD_DIST_BASE+0x0180 + (n * 4))
#define GICD_ISPENDR(n)         (GICD_DIST_BASE+0x0200 + (n * 4))
#define GICD_ICPENDR(n)         (GICD_DIST_BASE+0x0280 + (n * 4))
#define GICD_ISACTIVER(n)       (GICD_DIST_BASE+0x0300 + (n * 4))
#define GICD_ICACTIVER(n)       (GICD_DIST_BASE+0x0380 + (n * 4))
#define GICD_IPRIORITYR(n)      (GICD_DIST_BASE+0x0400 + (n * 4))
#define GICD_ITARGETSR(n)       (GICD_DIST_BASE+0x0800 + (n * 4))
#define GICD_ICFGR(n)           (GICD_DIST_BASE+0x0C00 + (n * 4))
#define GICD_SGIR               (GICD_DIST_BASE+0x0F00)
#define GICD_CPENDSGIR(n)       (GICD_DIST_BASE+0x0F10 + (n * 4))
#define GICD_SPENDSGIR(n)       (GICD_DIST_BASE+0x0F20 + (n * 4))
#define GICD_PIDR2V2            (GICD_DIST_BASE+0x0FE8)

//GIC CPU interface
#define GICC_CPU_BASE (GICC_ADDR+0x2000)

#define GICC_CTLR   (GICC_CPU_BASE+0x0000)
#define GICC_PMR    (GICC_CPU_BASE+0x0004)
#define GICC_BPR    (GICC_CPU_BASE+0x0008)
#define GICC_IAR    (GICC_CPU_BASE+0x000C)
#define GICC_EOIR   (GICC_CPU_BASE+0x0010)
#define GICC_ABPR   (GICC_CPU_BASE+0x001C)
#define GICC_AIAR   (GICC_CPU_BASE+0x0020)
#define GICC_AEOIR  (GICC_CPU_BASE+0x0024)

#define GICD_ITARGETSR_CORE0 (1 << 0)
#define GICD_ITARGETSR_CORE1 (1 << 1)
#define GICD_ITARGETSR_CORE2 (1 << 2)
#define GICD_ITARGETSR_CORE3 (1 << 3)

#define CTLR_EN_GRP0 (1 << 0)
#define CTLR_EN_GRP1 (1 << 1)

//Video core shared IRQs
#define SYSTEM_TIMER_IRQ_0 (0x60) //96
#define SYSTEM_TIMER_IRQ_1 (0x61) //97
#define SYSTEM_TIMER_IRQ_2 (0x62) //98
#define SYSTEM_TIMER_IRQ_3 (0x63) //99

//GICD_PIDR2V2
#define GIC_REV_OFFSET  0x4