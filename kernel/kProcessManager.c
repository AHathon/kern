#include "kernel/kProcessManager.h"

unsigned kProcessManager_FindFreeProcSpace() {
	unsigned i;
	for(i = 0; i < MAX_PROC; i++){
		if(!processTable[i]->PID) break;
	}
	return i;
}

void kProcessManager_Init() {
	kmemset(processTable, sizeof(kProcess*) * MAX_PROC);
}

void kProcessManager_AddProc(kProcess *proc)
{
	unsigned p = kProcessManager_FindFreeProcSpace();
	processTable[p] = proc;
	processTable[p]->PID = ++lastPID;
	processTable[p]->flags = proc->flags | IS_ACTIVE_PROC;
	kprintf("Added process: %s [%d]\n", processTable[p]->name, processTable[p]->PID);
}

void kProcessManager_KillProcess(unsigned ind) 
{
	if(ind >= MAX_PROC) return;
	kProcess_Destroy(processTable[ind]);
	processTable[ind] = 0;
}

void kProcessManager_PrintDebugProc() {
	for(int i = 0; i < MAX_PROC; i++){
		if(processTable[i]){
			kprintf(
				"Process:\n"
				"PID: %d\n",
				processTable[i]->PID
			);
		}
	}
}