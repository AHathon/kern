#include "kernel/kProcessManager.h"
#include "kernel/kThread.h"

void kProcessManager_Init() 
{
	kmemset((uint8_t*)processTable, sizeof(kProcess) * MAX_PROC);
	for(int i = 0; i < MAX_PROC; i++)
		processTable[i].flags = 0;
}

kProcess *kProcessManager_GetCurrentProcess()
{
	kThread *currThread = kScheduler_GetCurrentThread();
	return (kProcess *)currThread->parent;
}

void kProcessManager_CreateProcess(char *name, uint8_t *code, size_t codeSize, uint8_t isKernelProc)
{
	//Create proc in procTable
	unsigned p = ++lastPID;
	kstrcpy(processTable[p].name, name);
	processTable[p].PID = p;
	SET_BIT(processTable[p].flags, IS_ACTIVE_PROC);
	processTable[p].code.text.addr = (uintptr_t)KERN_VADDR_TO_PADDR(kMemAlloc(codeSize));
    processTable[p].code.text.size = codeSize;
	kmemcpy((uint8_t*)KERN_PADDR_TO_VADDR(processTable[p].code.text.addr), code, codeSize);

	//Create main thread and map mem 
	//[processes should all have the same virtual mapping give or take kern vaddr for ttrb0/1 routing]
	processTable[p].pageTables = (uintptr_t)kMemCalloc(PAGE_SIZE);	
	MMU_MapMemPages(processTable[p].pageTables, processTable[p].code.text.addr, USERLAND_VIRT_BASE, codeSize, isKernelProc);
	
	processTable[p].mainThread = kThread_Create(&processTable[p], (void*)(USERLAND_VIRT_BASE), 0x1000, isKernelProc ? THREAD_KERNEL : THREAD_USER);

	//set pagetable as phys addr
	processTable[p].pageTables = KERN_VADDR_TO_PADDR(processTable[p].pageTables);
	LOG("Page tables @ 00x%X\n", processTable[p].pageTables);

	processTable[p].threadCnt = 0;
	kmemset(processTable[p].threadsOwned, sizeof(kThread*) * MAX_THREADS_OWNED);
	processTable[p].threadsOwned[0] = processTable[p].mainThread; //TODO: cleaner impl
	processTable[p].threadCnt++;

	//Add main thread to scheduler
	kScheduler_AddThread(processTable[p].mainThread);
	LOGT("Added process: %s [pid:%d]\n", processTable[p].name, processTable[p].PID);
}

void kProcessManager_KillProcess(unsigned ind) 
{
	if(ind >= MAX_PROC) return;
	//Set process as inactive and set all threads to terminated
	CLR_BIT(processTable[ind].flags, IS_ACTIVE_PROC);
	while(processTable[ind].threadCnt)
	{
		processTable[ind].threadsOwned[processTable[ind].threadCnt-- - 1]->state = STATE_TERMINATED;
	}
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