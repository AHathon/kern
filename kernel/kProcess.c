#include "kernel/kProcess.h"

kProcess *kProcess_Create(char *name, uint8_t *code, size_t codeSize, void *entry)
{
    kProcess *proc = kMemAlloc(sizeof(kProcess));
    kstrcpy(proc->name, name);
    proc->PID = -1;
    proc->mainThread = kThread_Create(entry, 0x1000, THREAD_KERNEL);
    proc->code.text.addr = (uintptr_t)code;
    proc->code.text.size = codeSize;
}

void kProcess_Destroy(kProcess *proc)
{
    kThread_Destroy(proc->mainThread);
    kMemFree(proc, sizeof(kProcess));
}