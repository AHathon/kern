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
#include "libraries/services/smc.h"

void kMain(void)
{    
    uint64_t vbar;
    asm volatile ("mrs %0, vbar_el1" : "=r" (vbar));
    LOGT("Exception vector: 0x%X\n", vbar);
    LOGT("GIC [%s] (%d IRQs)\n", GIC_GetGicVersion(), GIC_GetGicMaxIRQs());
    LOGT("MMIO base: 0x%X\n", MMIO_ADDR);
    LOGT("Running at EL%d\n", GetCurrentEL());

    kMemManager_Init();
    kProcessManager_Init();
    kInitialProcess_Setup();

    kScheduler_Init();
    LOGT("Kernel init done!\n");
}
