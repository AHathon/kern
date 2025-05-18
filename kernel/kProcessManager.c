#include "kernel/kProcessManager.h"

unsigned FindFreeProcSpace() {
	unsigned i;
	for(i = 0; i < MAX_PROC; i++){
		if(!processTable[i].PID) break;
	}
	return i;
}

void InitProcessTable() {
	for(int i = 0; i < MAX_PROC; i++) {
		processTable[i].PID = 0;
		processTable[i].flags = 0;
	}
}

void CreateKProc(size_t stackSize, unsigned flags) {
	kprintf("Creating process\n");
	unsigned proc = FindFreeProcSpace();
	processTable[proc].PID = ++lastPID;
	processTable[proc].flags = flags | IS_ACTIVE_PROC;
	processTable[proc].memoryPool = PageAllocator_AllocPages(stackSize); //stack
	processTable[proc].memoryPoolSize = stackSize;
}

void KillProcess(unsigned ind) {
	if(ind >= MAX_PROC) return;
	processTable[ind].PID = 0;
	processTable[ind].flags = 0;
	if(processTable[ind].memoryPoolSize > 0)
		PageAllocator_FreePages(processTable[ind].memoryPool % PAGE_SIZE, processTable[ind].memoryPoolSize);
}

void PrintDebugProc() {
	for(int i = 0; i < MAX_PROC; i++){
		if(processTable[i].PID){
			kprintf(
				"Process:\n"
				"PID: %d\n"
				"Memory Pool Addr: %X\n",
				processTable[i].PID,
				processTable[i].memoryPool
			);
		}
	}
}

void startKIPs(){
	const char *start = &__kips_start;
	const char *end = &__kips_end;
	size_t kip_size = end - start;
    kprintf("Kip blob size: %X\n", kip_size);
}