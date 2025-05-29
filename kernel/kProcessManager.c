#include "kernel/kProcessManager.h"

unsigned kProcessManager_FindFreeProcSpace() {
	unsigned i;
	for(i = 0; i < MAX_PROC; i++){
		if(!processTable[i].PID) break;
	}
	return i;
}

void kProcessManager_Init() {
	for(int i = 0; i < MAX_PROC; i++) {
		processTable[i].PID = 0;
		processTable[i].flags = 0;
	}
}

void kProcessManager_CreateKProc(size_t stackSize, unsigned flags) {
	kprintf("Creating process\n");
	unsigned proc = kProcessManager_FindFreeProcSpace();
	processTable[proc].PID = ++lastPID;
	processTable[proc].flags = flags | IS_ACTIVE_PROC;
	//processTable[proc].memoryPool = PageAllocator_AllocPages(stackSize); //stack
	processTable[proc].memoryPoolSize = stackSize;
}

void kProcessManager_KillProcess(unsigned ind) {
	if(ind >= MAX_PROC) return;
	processTable[ind].PID = 0;
	processTable[ind].flags = 0;
	//if(processTable[ind].memoryPoolSize > 0)
	//	PageAllocator_FreePages(processTable[ind].memoryPool % PAGE_SIZE, processTable[ind].memoryPoolSize);
}

void kProcessManager_PrintDebugProc() {
	for(int i = 0; i < MAX_PROC; i++){
		if(processTable[i].PID){
			kprintf(
				"Process:\n"
				"PID: %d\n",
				processTable[i].PID
			);
		}
	}
}