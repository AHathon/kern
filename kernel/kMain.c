#include "libraries/types.h"

#include "kernel/memory/kMemoryManager.h"
#include "kernel/kProcessManager.h"
#include "kernel/exceptions.h"
#include "kernel/kScheduler.h"
#include "kernel/kInitialProcess.h"
#include "libraries/hardware/debug.h"
#include "libraries/hardware/gpio.h"
#include "libraries/hardware/cpu.h"

void kMain(uint64_t dtb_ptr32)
{
    uint64_t vbar;
    asm volatile ("mrs %0, vbar_el1" : "=r" (vbar));
    kprintf("Exception vector: %X\n", vbar);
    kprintf("MMIO base: %X\n", MMIO_BASE);
    kprintf("Running at EL%d\n", GetCurrentEL());

    kMemManager_Init();

    kProcessManager_Init();
    kScheduler_Init();

    kInitialProcess_Setup();
    kProcessManager_PrintDebugProc();
    
    kScheduler_Start();

    kprintf("Initialization done!\n");
}
