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

void CreateKProc(size_t memory, unsigned flags) {
	kprintf("Creating process\n");
	unsigned proc = FindFreeProcSpace();
	processTable[proc].PID = ++lastPID;
	processTable[proc].flags = flags | IS_ACTIVE_PROC;
	processTable[proc].heap = (uintptr_t)0;
	processTable[proc].heapSize = 0;
}

void KillProcess(unsigned ind) {
	if(ind >= MAX_PROC) return;
	processTable[ind].PID = 0;
	processTable[ind].flags = 0;
	if(processTable[ind].heapSize > 0)
		FreePages(processTable[ind].heap % PAGESIZE, processTable[ind].heapSize);
}

void PrintDebugProc() {
	for(int i = 0; i < MAX_PROC; i++){
		if(processTable[i].PID){
			kprintf(
				"Process:\n"
				"PID: %d\n"
				"Heap Addr: %X\n",
				processTable[i].PID,
				processTable[i].heap
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