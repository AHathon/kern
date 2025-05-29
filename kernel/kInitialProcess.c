#include "kernel/kInitialProcess.h"

void kInitialProcess_Setup()
{
    const char *start = &__kips_start;
	const char *end = &__kips_end;
	size_t kip_size = end - start;
    kprintf("Kip blob size: %X\n", kip_size);

    uintptr_t off = 0;
    while(off < kip_size)
    {
        KipHeader *hdr = (KipHeader*)(start + off);
        kprintf("Module: %s\n", hdr->magic);
        kprintf("Size: %X\n", hdr->totalSize);
        uintptr_t entry = off + hdr->headerSize;
        size_t codeSize = hdr->codeSize;
        
        uint8_t *code = (uint8_t*)kMemAlloc(codeSize);
        kmemcpy(code, (uint8_t*)entry, codeSize);
        kProcess *proc = kProcess_Create(hdr->magic, code, codeSize, code);
        kProcessManager_AddProc(proc);

        //Add main thread to scheduler
        kScheduler_AddThread(proc->mainThread);

        off += hdr->totalSize;
    }
}