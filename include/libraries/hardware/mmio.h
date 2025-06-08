#pragma once

#include "libraries/hardware/constants.h"

//Legacy interrupt controller
#define IRQ_BASIC_PENDING	(MMIO_ADDR+0x0000B200)
#define IRQ_PENDING_1		(MMIO_ADDR+0x0000B204)
#define IRQ_PENDING_2		(MMIO_ADDR+0x0000B208)
#define FIQ_CONTROL		    (MMIO_ADDR+0x0000B20C)
#define ENABLE_IRQS_1		(MMIO_ADDR+0x0000B210)
#define ENABLE_IRQS_2		(MMIO_ADDR+0x0000B214)
#define ENABLE_BASIC_IRQS	(MMIO_ADDR+0x0000B218)
#define DISABLE_IRQS_1		(MMIO_ADDR+0x0000B21C)
#define DISABLE_IRQS_2		(MMIO_ADDR+0x0000B220)
#define DISABLE_BASIC_IRQS	(MMIO_ADDR+0x0000B224)

//Video core mailbox
#define VC_MBOX_BASE        (MMIO_ADDR+0x0000B880)

#define VC_MBOX_READ        ((volatile uint32_t*)(VC_MBOX_BASE+0x00000000))
#define VC_MBOX_POLL        ((volatile uint32_t*)(VC_MBOX_BASE+0x00000010))
#define VC_MBOX_SENDER      ((volatile uint32_t*)(VC_MBOX_BASE+0x00000014))
#define VC_MBOX_STATUS      ((volatile uint32_t*)(VC_MBOX_BASE+0x00000018))
#define VC_MBOX_CONFIG      ((volatile uint32_t*)(VC_MBOX_BASE+0x0000001C))
#define VC_MBOX_WRITE       ((volatile uint32_t*)(VC_MBOX_BASE+0x00000020))

#define MBOX_TAG_SETPOWER   0x28001,
#define MBOX_TAG_SETCLKRATE  0x38002,

#define MBOX_RESPONSE       0x80000000
#define MBOX_FULL           0x80000000
#define MBOX_EMPTY          0x40000000

//GPIO
#define GPFSEL0         ((volatile uint32_t*)(GPIO_BASE+0x0000))
#define GPFSEL1         ((volatile uint32_t*)(GPIO_BASE+0x0004))
#define GPFSEL2         ((volatile uint32_t*)(GPIO_BASE+0x0008))
#define GPFSEL3         ((volatile uint32_t*)(GPIO_BASE+0x000C))
#define GPFSEL4         ((volatile uint32_t*)(GPIO_BASE+0x0010))
#define GPFSEL5         ((volatile uint32_t*)(GPIO_BASE+0x0014))
#define GPSET0          ((volatile uint32_t*)(GPIO_BASE+0x001C))
#define GPSET1          ((volatile uint32_t*)(GPIO_BASE+0x0020))
#define GPCLR0          ((volatile uint32_t*)(GPIO_BASE+0x0028))
#define GPCLR1          ((volatile uint32_t*)(GPIO_BASE+0x002C))
#define GPLEV0          ((volatile uint32_t*)(GPIO_BASE+0x0034))
#define GPLEV1          ((volatile uint32_t*)(GPIO_BASE+0x0038))
#define GPEDS0          ((volatile uint32_t*)(GPIO_BASE+0x0040))
#define GPEDS1          ((volatile uint32_t*)(GPIO_BASE+0x0044))
#define GPHEN0          ((volatile uint32_t*)(GPIO_BASE+0x0064))
#define GPHEN1          ((volatile uint32_t*)(GPIO_BASE+0x0068))
#define GPPUD           ((volatile uint32_t*)(GPIO_BASE+0x0094))
#define GPPUDCLK0       ((volatile uint32_t*)(GPIO_BASE+0x0098))
#define GPPUDCLK1       ((volatile uint32_t*)(GPIO_BASE+0x009C))

#define FUNC_IP  0
#define FUNC_OP  1
#define FUNC_A0  4
#define FUNC_A1  5
#define FUNC_A2  6
#define FUNC_A3  7
#define FUNC_A4  3
#define FUNC_A5  2

//UART0
#define UART0_DR     ((volatile uint32_t*)(UART0_BASE + 0x00))
#define UART0_FR     ((volatile uint32_t*)(UART0_BASE + 0x18))
#define UART0_IBRD   ((volatile uint32_t*)(UART0_BASE + 0x24))
#define UART0_FBRD   ((volatile uint32_t*)(UART0_BASE + 0x28))
#define UART0_LCRH   ((volatile uint32_t*)(UART0_BASE + 0x2C))
#define UART0_CR     ((volatile uint32_t*)(UART0_BASE + 0x30))
#define UART0_IMSC   ((volatile uint32_t*)(UART0_BASE + 0x38))
#define UART0_ICR    ((volatile uint32_t*)(UART0_BASE + 0x44))

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
#define CONTROL_REGISTER                (ARM_LOCAL_ADDR+0x00)
#define CORE0_TIMER_INTERRUPT_CTRL      (ARM_LOCAL_ADDR+0x40)
#define CORE1_TIMER_INTERRUPT_CTRL      (ARM_LOCAL_ADDR+0x44)
#define CORE2_TIMER_INTERRUPT_CTRL      (ARM_LOCAL_ADDR+0x48)
#define CORE3_TIMER_INTERRUPT_CTRL      (ARM_LOCAL_ADDR+0x4C)
#define CORE0_MAILBOX_INTERRUPT_CTRL    (ARM_LOCAL_ADDR+0x50)
#define CORE1_MAILBOX_INTERRUPT_CTRL    (ARM_LOCAL_ADDR+0x54)
#define CORE2_MAILBOX_INTERRUPT_CTRL    (ARM_LOCAL_ADDR+0x58)
#define CORE3_MAILBOX_INTERRUPT_CTRL    (ARM_LOCAL_ADDR+0x5C)
#define CORE0_IRQ_SOURCE                (ARM_LOCAL_ADDR+0x60)
#define CORE1_IRQ_SOURCE                (ARM_LOCAL_ADDR+0x64)
#define CORE2_IRQ_SOURCE                (ARM_LOCAL_ADDR+0x68)
#define CORE3_IRQ_SOURCE                (ARM_LOCAL_ADDR+0x6C)
#define CORE0_FIQ_SOURCE                (ARM_LOCAL_ADDR+0x70)
#define CORE1_FIQ_SOURCE                (ARM_LOCAL_ADDR+0x74)
#define CORE2_FIQ_SOURCE                (ARM_LOCAL_ADDR+0x78)
#define CORE3_FIQ_SOURCE                (ARM_LOCAL_ADDR+0x7C)

//BCM timer
#define SYS_TIMER_BASE  (MMIO_ADDR + 0x3000)
#define SYS_TIMER_CS    (SYS_TIMER_BASE + 0x00)
#define SYS_TIMER_CS_M0 (1 << 0)
#define SYS_TIMER_CS_M1 (1 << 1)
#define SYS_TIMER_CS_M2 (1 << 2)
#define SYS_TIMER_CS_M3 (1 << 3)
#define SYS_TIMER_CLO   (SYS_TIMER_BASE + 0x04)
#define SYS_TIMER_CHI   (SYS_TIMER_BASE + 0x08)
#define SYS_TIMER_C0    (SYS_TIMER_BASE + 0x0C)
#define SYS_TIMER_C1    (SYS_TIMER_BASE + 0x10)
#define SYS_TIMER_C2    (SYS_TIMER_BASE + 0x14)
#define SYS_TIMER_C3    (SYS_TIMER_BASE + 0x18)

//GIC Distributer
#define GICD_DIST_BASE (GICC_ADDR+0x00001000)

#define GICD_CTLR               (GICD_DIST_BASE+0x00000000)
#define GICD_TYPER              (GICD_DIST_BASE+0x00000004)
#define GICD_IGROUPR(n)         (GICD_DIST_BASE+0x00000080 + (n * 4))
#define GICD_ISENABLER(n)       (GICD_DIST_BASE+0x00000100 + (n * 4))
#define GICD_ISPENDR(n)         (GICD_DIST_BASE+0x00000200 + (n * 4))
#define GICD_IPRIORITYR(n)      (GICD_DIST_BASE+0x00000400 + (n * 4))
#define GICD_ITARGETSR(n)       (GICD_DIST_BASE+0x00000800 + (n * 4))
#define GICD_ICFGR(n)           (GICD_DIST_BASE+0x00000C00 + (n * 4))
#define GICD_SGIR               (GICD_DIST_BASE+0x00000F00)
#define GICD_CPENDSGIR(n)       (GICD_DIST_BASE+0x00000F10 + (n * 4))
#define GICD_SPENDSGIR(n)       (GICD_DIST_BASE+0x00000F20 + (n * 4))
#define GICD_PIDR2V2            (GICD_DIST_BASE+0x00000FE8)

//GIC CPU interface
#define GICC_CPU_BASE (GICC_ADDR+0x00002000)

#define GICC_CTLR   (GICC_CPU_BASE+0x00000000)
#define GICC_PMR    (GICC_CPU_BASE+0x00000004)
#define GICC_BPR    (GICC_CPU_BASE+0x00000008)
#define GICC_IAR    (GICC_CPU_BASE+0x0000000C)
#define GICC_EOIR   (GICC_CPU_BASE+0x00000010)

//Local IRQ
#define LOCAL_TIMER_IRQ_PS      29
#define LOCAL_TIMER_IRQ_PNS     30

//Video core shared IRQs
#define SYSTEM_TIMER_IRQ_0 (0x60) //96
#define SYSTEM_TIMER_IRQ_1 (0x61) //97
#define SYSTEM_TIMER_IRQ_2 (0x62) //98
#define SYSTEM_TIMER_IRQ_3 (0x63) //99

#define GIC_PRI_HIGHEST_SECURE 0x00
#define GIC_PRI_HIGHEST_NONSECURE 0x80
#define GIC_PRI_DEFAULT 0xA0

#define GICD_ITARGETSR_CORE0 (1 << 0)
#define GICD_ITARGETSR_CORE1 (1 << 1)
#define GICD_ITARGETSR_CORE2 (1 << 2)
#define GICD_ITARGETSR_CORE3 (1 << 3)

//GICD_PIDR2V2
#define GIC_REV_OFFSET  0x4