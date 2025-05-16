#include "libraries/types.h"

#include "kernel/mmu.h"
#include "kernel/gpio.h"
#include "kernel/debug.h"
#include "kernel/memory/kMemoryManager.h"
#include "kernel/kProcessManager.h"
#include "kernel/exceptions.h"
#include "kernel/debug.h"
#include "kernel/cpu.h"
#include "kernel/kScheduler.h"

void kMain(uint64_t dtb_ptr32){

    kprintf("MMIO base: %X\n", MMIO_BASE);
    kprintf("Running at EL%d\n", GetCurrentEL());

    kMemManager_Init();
	
	InitProcessTable();
    startKIPs();
	
    kScheduler_init();

	CreateKProc(0x1000, 0);
	PrintDebugProc();
    
    kprintf("Initialization done!\n");
}
