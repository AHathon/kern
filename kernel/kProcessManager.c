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

void kProcessManager_CreateProcess(char *name, uint8_t *code, size_t codeSize, uint8_t isKernelProc)
{
	//Create proc in procTable
	unsigned p = kProcessManager_FindFreeProcSpace();
	kstrcpy(processTable[p].name, name);
	processTable[p].PID = ++lastPID;
	processTable[p].pageTables = (uintptr_t)kMemAlloc(3 * PAGE_SIZE);
	processTable[p].flags |= IS_ACTIVE_PROC;
	processTable[p].code.text.addr = (uintptr_t)kMemAlloc(codeSize);
    processTable[p].code.text.size = codeSize;
	kmemcpy((uint8_t*)(processTable[p].code.text.addr), code, codeSize);
	
	//Add main thread to scheduler
    kScheduler_AddThread(processTable[p].mainThread);
	processTable[p].mainThread = kThread_Create(&processTable[p], (void*)(processTable[p].code.text.addr), 0x1000, isKernelProc ? THREAD_KERNEL : THREAD_USER);
	if(!isKernelProc)
		MMU_mapUserMem(processTable[p].pageTables, processTable[p].code.text.addr - KERNEL_VIRT_BASE, USERLAND_VIRT_BASE, codeSize);
	LOG("Added process: %s [pid:%d]\n", processTable[p].name, processTable[p].PID);
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
			LOG(
				"Process:\n"
				"PID: %d\n",
				processTable[i].PID
			);
		}
	}
}