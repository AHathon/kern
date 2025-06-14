#include "kernel/memory/kMemoryManager.h"
#include "kernel/kProcessManager.h"
#include "kernel/kScheduler.h"
#include "kernel/kInitialProcess.h"
#include "libraries/hardware/debug.h"
#include "libraries/hardware/gpio.h"
#include "libraries/hardware/cpu.h"
#include "libraries/hardware/mmio_vars.h"
#include "libraries/hardware/constants.h"
#include "libraries/hardware/framebuffer.h"
#include "libraries/ui/terminal.h"
#include "libraries/hardware/gic.h"
#include "libraries/types.h"

void kMain()
{
    GicDisable();
    //Only C1/C3 should be used for OS
    //GicRouteIRQ(SYSTEM_TIMER_IRQ_1);
    //GicRouteIRQ(SYSTEM_TIMER_IRQ_3);
    GicRouteIRQ(LOCAL_TIMER_IRQ_PNS);
    GicEnable();
    
    uint64_t vbar;
    asm volatile ("mrs %0, vbar_el1" : "=r" (vbar));
    LOGT("Exception vector: %X\n", vbar);
    LOGT("MMIO base: %X\n", MMIO_ADDR);
    LOGT("Running at EL%d\n", GetCurrentEL());

    kMemManager_Init();
    kProcessManager_Init();
    kInitialProcess_Setup();

    kScheduler_Init();
    LOGT("Initialization done!\n");
}
