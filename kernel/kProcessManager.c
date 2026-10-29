#include "kernel/kProcessManager.h"
#include "kernel/kThread.h"
#include "kernel/memory/kMemoryMap.h"
#include "kernel/kMutex.h"
#include "kernel/kIpc.h"
#include "libraries/libc/string.h"

static kProcess processTable[MAX_PROC];
static unsigned lastPID = 0;
static kMutex_t *procMan_mutex = 0;

void kProcessManager_Init() 
{
	procMan_mutex = kMutex_alloc();
	ASSERT(procMan_mutex);
	kMutex_Init(procMan_mutex);

	memset((uint8_t*)processTable, sizeof(kProcess) * MAX_PROC);
	for(int i = 0; i < MAX_PROC; i++)
		processTable[i].flags = 0;
}

kProcess *kProcessManager_GetCurrentProcess()
{
	kThread *currThread = kScheduler_GetCurrentThread();
	return (kProcess *)currThread->parent;
}

int64_t kProcessManager_CreateProcess(char *name, uint8_t *code, size_t codeSize, uint8_t isKernelProc, uint8_t start)
{
	kMutex_Lock(procMan_mutex);

	size_t userStackSize = PAGE_SIZE * 2;

	if(lastPID + 1 >= MAX_PROC)
	{
		kMutex_Unlock(procMan_mutex);
		return -1;
	}

	uint8_t *codeMem = kMemAlloc(codeSize);
	uint8_t *pageTables = kMemCalloc(PAGE_SIZE);
	if(!codeMem || !pageTables)
	{
		if(codeMem) 
			kMemFree(codeMem, codeSize);
		if(pageTables) 
			kMemFree(pageTables, PAGE_SIZE);
		kMutex_Unlock(procMan_mutex);
		return -1;
	}

	//Create proc in procTable
	unsigned p = ++lastPID;
	size_t n = 0;
	for(; name[n] && n < sizeof(processTable[p].name) - 1; n++)
		processTable[p].name[n] = name[n];
	processTable[p].name[n] = 0;
	processTable[p].PID = p;
	SET_BIT(processTable[p].flags, IS_ACTIVE_PROC);
	processTable[p].code.text.addr = (uintptr_t)KERN_VADDR_TO_PADDR(codeMem);
    processTable[p].code.text.size = codeSize;
	memcpy(codeMem, code, codeSize);

	//Create main thread and map mem
	//[processes should all have the same virtual mapping give or take kern vaddr for ttrb0/1 routing]
	processTable[p].pageTables = (uintptr_t)pageTables;
	uint64_t flags = isKernelProc ? ATTR_KERNEL : 0;
	MMU_MapMemPages(processTable[p].pageTables, processTable[p].code.text.addr, USERLAND_VIRT_BASE, codeSize, flags);
	
	processTable[p].mainThread = kThread_Create(&processTable[p], (void*)(USERLAND_VIRT_BASE), userStackSize, isKernelProc ? THREAD_KERNEL : THREAD_USER);

	//set pagetable as phys addr
	processTable[p].pageTables = KERN_VADDR_TO_PADDR(processTable[p].pageTables);
	LOG("Page tables @ 0x%X\n", processTable[p].pageTables);

	processTable[p].threadCnt = 0;
	memset((uint8_t*)processTable[p].threadsOwned, sizeof(kThread*) * MAX_THREADS_OWNED);
	processTable[p].threadsOwned[0] = processTable[p].mainThread; //TODO: cleaner impl
	processTable[p].threadCnt++;

	//Add main thread to scheduler
	if(start)
		kProcessManager_StartProcess(p);
	LOGT("Added process: %s [pid:%d]\n", processTable[p].name, processTable[p].PID);

	kMutex_Unlock(procMan_mutex);
	return p;
}

void kProcessManager_KillProcess(unsigned ind) 
{
	if(ind >= MAX_PROC) return;
	//Set process as inactive and set all threads to terminated
	CLR_BIT(processTable[ind].flags, IS_ACTIVE_PROC);
	kIpc_ProcessExit(&processTable[ind]);
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
kProcess *kProcessManager_GetProcess(int64_t pid)
{
	if(pid < 1 || pid > lastPID || !(processTable[pid].flags & IS_ACTIVE_PROC))
		return 0;
	return &processTable[pid];
}

int64_t kProcessManager_StartProcess(int64_t pid)
{
	kProcess *proc = kProcessManager_GetProcess(pid);
	if(!proc || (proc->flags & IS_STARTED_PROC))
		return -1;

	SET_BIT(proc->flags, IS_STARTED_PROC);
	kScheduler_AddThread(proc->mainThread);
	return 0;
}
