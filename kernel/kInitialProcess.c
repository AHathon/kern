#include "kernel/kInitialProcess.h"
#include "kernel/kProcessManager.h"
#include "kernel/kScheduler.h"
#include "kernel/kThread.h"
#include "libraries/hardware/debug.h"

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
        kProcessManager_CreateProcess(hdr->magic, code, codeSize, code);

        off += hdr->totalSize;
    }
}