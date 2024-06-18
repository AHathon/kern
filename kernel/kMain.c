#include "libraries/types.h"

#include "kernel/mmu.h"
#include "kernel/gpio.h"
#include "kernel/debug.h"
#include "kernel/kMemoryManager.h"
#include "kernel/kProcessManager.h"
#include "kernel/exceptions.h"
#include "kernel/debug.h"

void kMain(uint64_t dtb_ptr32){
    init_uart1();
    
    kprintf("Initializing MMU\n");
    setupMMU();
    
    kprintf("Initializing memory manager\n");
    InitMemoryManager();
	
	//kprintf("Allocating kernel heap\n");
	
    kprintf("Setting exception vectors\n");
    SetExceptionVecTbl(&arm64_excep_vec_tbl);

    kprintf("Starting kernel internal processes\n");

	InitProcessTable();
    startKIPs();
	
	CreateKProc(0x10000, 0);
	PrintDebugProc();
	
    kprintf("Initialization done!\n");
    while (1)
        cpuSleep(); //sleep and wait on interrupt
}
