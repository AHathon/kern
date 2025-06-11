#pragma once

#define NR_SYSCALLS 4

#define MAIN_DISP_WIDTH     800
#define MAIN_DISP_HEIGHT    480

#define SCHEDULE_TIMER_INTERVAL 1000000UL  // 1 million µs = 1s

#define EC_UNKNOWN              0b000000
#define EC_TRAPPED_WF           0b000001
#define EC_ILLEGAL_EXECUTION    0b001110
#define EC_SVC_32               0b010001
#define EC_SVC_64               0b010101
#define EC_DATA_ABORT_NC_EL     0b100101

#define KERNEL_VIRT_BASE        0xFFFFFF8000000000ULL
#define USERLAND_VIRT_BASE      0x01000000
#define KERN_VADDR_TO_PADDR(x)  (x - KERNEL_VIRT_BASE)
#define KERN_PADDR_TO_VADDR(x)  (x + KERNEL_VIRT_BASE)

#define MMIO_BASE               0xFE000000
#define ARM_LOCAL_BASE          0xFF800000
#define GIC_BASE                0xFF840000
#define RAM_PADDR               0x01000000

#define GPIO_BASE  (MMIO_ADDR + 0x200000)
#define UART0_BASE (MMIO_ADDR + 0x201000)
#define UART2_BASE (MMIO_ADDR + 0x201400)
#define UART3_BASE (MMIO_ADDR + 0x201600)
#define UART4_BASE (MMIO_ADDR + 0x201800)
#define UART5_BASE (MMIO_ADDR + 0x201A00)

#define UART1_BASE (MMIO_ADDR + 0x210000) //Mini uart