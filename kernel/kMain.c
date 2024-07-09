#include "libraries/types.h"

#include "kernel/mmu.h"
#include "kernel/gpio.h"
#include "kernel/debug.h"
#include "kernel/memory/kMemoryManager.h"
#include "kernel/kProcessManager.h"
#include "kernel/exceptions.h"
#include "kernel/debug.h"
#include "kernel/cpu.h"

void kMain(uint64_t dtb_ptr32){
    UART1_Init();
    MMU_Init();

    kprintf("Running at EL%d\n", GetCurrentEL());

    kMemManager_Init();
	
	InitProcessTable();
    startKIPs();
	
	CreateKProc(0x10000, 0);
	PrintDebugProc();
    
    kprintf("Initialization done!\n");
}
