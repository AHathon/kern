#pragma once

#include "libraries/hardware/mapping.h"

#define NR_SYSCALLS 3

#define EC_UNKNOWN              0b000000
#define EC_TRAPPED_WF           0b000001
#define EC_ILLEGAL_EXECUTION    0b001110
#define EC_SVC_32               0b010001
#define EC_SVC_64               0b010101
#define EC_DATA_ABORT_NC_EL     0b100101

#define IRQ_BASIC_PENDING	(MMIO_BASE+0x0000B200)
#define IRQ_PENDING_1		(MMIO_BASE+0x0000B204)
#define IRQ_PENDING_2		(MMIO_BASE+0x0000B208)
#define FIQ_CONTROL		    (MMIO_BASE+0x0000B20C)
#define ENABLE_IRQS_1		(MMIO_BASE+0x0000B210)
#define ENABLE_IRQS_2		(MMIO_BASE+0x0000B214)
#define ENABLE_BASIC_IRQS	(MMIO_BASE+0x0000B218)
#define DISABLE_IRQS_1		(MMIO_BASE+0x0000B21C)
#define DISABLE_IRQS_2		(MMIO_BASE+0x0000B220)
#define DISABLE_BASIC_IRQS	(MMIO_BASE+0x0000B224)

#define GPFSEL0         ((volatile unsigned int*)(GPIO_BASE+0x0000))
#define GPFSEL1         ((volatile unsigned int*)(GPIO_BASE+0x0004))
#define GPFSEL2         ((volatile unsigned int*)(GPIO_BASE+0x0008))
#define GPFSEL3         ((volatile unsigned int*)(GPIO_BASE+0x000C))
#define GPFSEL4         ((volatile unsigned int*)(GPIO_BASE+0x0010))
#define GPFSEL5         ((volatile unsigned int*)(GPIO_BASE+0x0014))
#define GPSET0          ((volatile unsigned int*)(GPIO_BASE+0x001C))
#define GPSET1          ((volatile unsigned int*)(GPIO_BASE+0x0020))
#define GPCLR0          ((volatile unsigned int*)(GPIO_BASE+0x0028))
#define GPLEV0          ((volatile unsigned int*)(GPIO_BASE+0x0034))
#define GPLEV1          ((volatile unsigned int*)(GPIO_BASE+0x0038))
#define GPEDS0          ((volatile unsigned int*)(GPIO_BASE+0x0040))
#define GPEDS1          ((volatile unsigned int*)(GPIO_BASE+0x0044))
#define GPHEN0          ((volatile unsigned int*)(GPIO_BASE+0x0064))
#define GPHEN1          ((volatile unsigned int*)(GPIO_BASE+0x0068))
#define GPPUD           ((volatile unsigned int*)(GPIO_BASE+0x0094))
#define GPPUDCLK0       ((volatile unsigned int*)(GPIO_BASE+0x0098))
#define GPPUDCLK1       ((volatile unsigned int*)(GPIO_BASE+0x009C))

#define UART1_ENABLE    ((volatile unsigned int *)(UART_BASE+0x5004))
#define UART1_IO        ((volatile unsigned int *)(UART_BASE+0x5040))
#define UART1_IER       ((volatile unsigned int *)(UART_BASE+0x5044))
#define UART1_IIR       ((volatile unsigned int *)(UART_BASE+0x5048))
#define UART1_LCR       ((volatile unsigned int *)(UART_BASE+0x504C))
#define UART1_MCR       ((volatile unsigned int *)(UART_BASE+0x5050))
#define UART1_LSR       ((volatile unsigned int *)(UART_BASE+0x5054))
#define UART1_MSR       ((volatile unsigned int *)(UART_BASE+0x5058))
#define UART1_SCRATCH   ((volatile unsigned int *)(UART_BASE+0x505C))
#define UART1_CNTL      ((volatile unsigned int *)(UART_BASE+0x5060))
#define UART1_STAT      ((volatile unsigned int *)(UART_BASE+0x5064))
#define UART1_BAUD      ((volatile unsigned int *)(UART_BASE+0x5068))

#define TIMERS_INTERUPTS_BASE 0x40000000

#define CONTROL_REGISTER                (TIMERS_INTERUPTS_BASE+0x00)
#define CORE0_TIMER_INTERRUPT_CTRL      (TIMERS_INTERUPTS_BASE+0x40)
#define CORE1_TIMER_INTERRUPT_CTRL      (TIMERS_INTERUPTS_BASE+0x44)
#define CORE2_TIMER_INTERRUPT_CTRL      (TIMERS_INTERUPTS_BASE+0x48)
#define CORE3_TIMER_INTERRUPT_CTRL      (TIMERS_INTERUPTS_BASE+0x4C)
#define CORE0_MAILBOX_INTERRUPT_CTRL    (TIMERS_INTERUPTS_BASE+0x50)
#define CORE1_MAILBOX_INTERRUPT_CTRL    (TIMERS_INTERUPTS_BASE+0x54)
#define CORE2_MAILBOX_INTERRUPT_CTRL    (TIMERS_INTERUPTS_BASE+0x58)
#define CORE3_MAILBOX_INTERRUPT_CTRL    (TIMERS_INTERUPTS_BASE+0x5C)
#define CORE0_IRQ_SOURCE                (TIMERS_INTERUPTS_BASE+0x60)
#define CORE1_IRQ_SOURCE                (TIMERS_INTERUPTS_BASE+0x64)
#define CORE2_IRQ_SOURCE                (TIMERS_INTERUPTS_BASE+0x68)
#define CORE3_IRQ_SOURCE                (TIMERS_INTERUPTS_BASE+0x6C)
#define CORE0_FIQ_SOURCE                (TIMERS_INTERUPTS_BASE+0x70)
#define CORE1_FIQ_SOURCE                (TIMERS_INTERUPTS_BASE+0x74)
#define CORE2_FIQ_SOURCE                (TIMERS_INTERUPTS_BASE+0x78)
#define CORE3_FIQ_SOURCE                (TIMERS_INTERUPTS_BASE+0x7C)

#define LOCAL_TIMER_IRQ		(1 << 11)

#define SYSTEM_TIMER_IRQ_0	(1 << 0)
#define SYSTEM_TIMER_IRQ_1	(1 << 1)
#define SYSTEM_TIMER_IRQ_2	(1 << 2)
#define SYSTEM_TIMER_IRQ_3	(1 << 3)