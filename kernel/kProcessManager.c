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
		processTable[i].flags = IS_INACTIVE_PROC;
	}
}

void CreateKProc(size_t memory, unsigned flags) {
	kprintf("Creating process\n");
	unsigned proc = FindFreeProcSpace();
	processTable[proc].PID = ++lastPID;
	processTable[proc].flags = flags | IS_ACTIVE_PROC;
	processTable[proc].vaddr = kMemAlloc(memory);
}

void KillProcess(unsigned ind) {
	if(ind >= MAX_PROC) return;
	processTable[ind].PID = 0;
	processTable[ind].flags = IS_INACTIVE_PROC;
	kMemFree(processTable[ind].vaddr);
}

void PrintDebugProc() {
	for(int i = 0; i < MAX_PROC; i++){
		if(processTable[i].PID){
			kprintf(
				"Process:\n"
				"PID: %d\n"
				"Memory Addr: %X\n",
				processTable[i].PID,
				processTable[i].vaddr
			);
		}
	}
}

void startKIPs(){
	const char *start = &__kips_start;
	const char *end = &__kips_end;
	size_t kip_size = &end - &start;
    kprintf("Kip size: %X\n", kip_size);
	kprintf("Kip start: %X\n", start);
}