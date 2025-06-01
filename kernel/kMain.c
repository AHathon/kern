#include "libraries/types.h"

#include "kernel/memory/kMemoryManager.h"
#include "kernel/kProcessManager.h"
#include "kernel/exceptions.h"
#include "kernel/kScheduler.h"
#include "kernel/kInitialProcess.h"
#include "libraries/hardware/debug.h"
#include "libraries/hardware/gpio.h"
#include "libraries/hardware/cpu.h"
#include "libraries/hardware/mmio_vars.h"
#include "libraries/hardware/gic.h"

void kMain(uint64_t dtb_ptr32)
{
    GicEnable();
    //Only C1/C3 should be used for OS
    //GicRouteIRQ(SYSTEM_TIMER_IRQ_1);
    //GicRouteIRQ(SYSTEM_TIMER_IRQ_3);
    GicRouteIRQ(LOCAL_TIMER_IRQ_PNS);
    
    uint64_t vbar;
    asm volatile ("mrs %0, vbar_el1" : "=r" (vbar));
    kprintf("Exception vector: %X\n", vbar);
    kprintf("MMIO base: %X\n", MMIO_ADDR);
    kprintf("Running at EL%d\n", GetCurrentEL());

    kMemManager_Init();

    kProcessManager_Init();
    kScheduler_Init();

    kInitialProcess_Setup();
    kProcessManager_PrintDebugProc();
    
    kScheduler_Start();

    kprintf("Initialization done!\n");
}
