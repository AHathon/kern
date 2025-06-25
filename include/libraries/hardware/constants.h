#pragma once

#define NR_SYSCALLS 256

#define MAIN_DISP_WIDTH     800
#define MAIN_DISP_HEIGHT    480

#define SCHEDULE_TIMER_INTERVAL 1000000UL  // 1 million µs = 1s

#ifdef PI3
    #define OSC_FREQ        19200000
#else
    #define OSC_FREQ		54000000
#endif

#define EC_UNKNOWN              0x00
#define EC_TRAPPED_WF           0x01
#define EC_ILLEGAL_EXECUTION    0x0E
#define EC_SVC_32               0x11
#define EC_HVC_32               0x12
#define EC_SMC_32               0x13
#define EC_SVC_64               0x15
#define EC_HVC_64               0x16
#define EC_SMC_64               0x17
#define EC_DATA_ABORT_EL0       0x24
#define EC_DATA_ABORT_EL1       0x25

//Local IRQ
#define LOCAL_TIMER_IRQ_PS      29
#define LOCAL_TIMER_IRQ_PNS     30

#ifdef PI3
    #define MMIO_BASE           0x3F000000
#else
    #define MMIO_BASE           0xFE000000
#endif
#define ARM_LOCAL_BASE          0xFF800000
#define GIC_BASE                0xFF840000

#define GPIO_BASE  (MMIO_ADDR + 0x200000)
#define UART0_BASE (MMIO_ADDR + 0x201000)
#define UART2_BASE (MMIO_ADDR + 0x201400)
#define UART3_BASE (MMIO_ADDR + 0x201600)
#define UART4_BASE (MMIO_ADDR + 0x201800)
#define UART5_BASE (MMIO_ADDR + 0x201A00)

#define UART1_BASE (MMIO_ADDR + 0x210000) //Mini uart