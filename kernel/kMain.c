#include "libraries/types.h"

#include "kernel/mmu.h"
#include "kernel/gpio.h"
#include "kernel/debug.h"
#include "kernel/memory/kMemoryManager.h"
#include "kernel/kProcessManager.h"
#include "kernel/exceptions.h"
#include "kernel/debug.h"

void kMain(uint64_t dtb_ptr32){
    UART1_Init();
    MMU_Init();

    kMemManager_Init();
	
    ExceptionVector_Init(&arm64_excep_vec_tbl);

	InitProcessTable();
    startKIPs();
	
	CreateKProc(0x10000, 0);
	PrintDebugProc();
	
    kprintf("Initialization done!\n");
    while (1)
        cpuSleep(); //sleep and wait on interrupt
}
