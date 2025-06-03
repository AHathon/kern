#include "kernel/kProcessManager.h"
#include "kernel/kThread.h"

unsigned kProcessManager_FindFreeProcSpace() {
	unsigned i;
	for(i = 0; i < MAX_PROC; i++){
		if(!(processTable[i].flags & IS_ACTIVE_PROC)) break;
	}
	return i;
}

void kProcessManager_Init() {
	kmemset((uint8_t*)processTable, sizeof(kProcess) * MAX_PROC);
	for(int i = 0; i < MAX_PROC; i++)
		processTable[i].flags = 0;
}

void kProcessManager_CreateProcess(char *name, uint8_t *code, size_t codeSize, void *entry)
{
	//Create proc in procTable
	unsigned p = kProcessManager_FindFreeProcSpace();
	kstrcpy(processTable[p].name, name);
	processTable[p].PID = ++lastPID;
	processTable[p].mainThread = kThread_Create(&processTable[p], entry, 0x1000, THREAD_KERNEL);
	processTable[p].flags |= IS_ACTIVE_PROC;
	processTable[p].code.text.addr = (uintptr_t)code;
    processTable[p].code.text.size = codeSize;
	
	//Add main thread to scheduler
    kScheduler_AddThread(processTable[p].mainThread);
	kprintf("Added process: %s [pid:%d]\n", processTable[p].name, processTable[p].PID);
}

void kProcessManager_KillProcess(unsigned ind) 
{
	if(ind >= MAX_PROC) return;
	kThread_Destroy(processTable[ind].mainThread);
	processTable[ind].flags = 0;
}

void kProcessManager_PrintDebugProc() {
	for(int i = 0; i < MAX_PROC; i++){
		if(processTable[i].flags & IS_ACTIVE_PROC){
			kprintf(
				"Process:\n"
				"PID: %d\n",
				processTable[i].PID
			);
		}
	}
}