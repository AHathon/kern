#include "kernel/kInitialProcess.h"
#include "kernel/kProcessManager.h"
#include "kernel/kScheduler.h"
#include "kernel/kThread.h"
#include "libraries/hardware/debug.h"

void kInitialProcess_Setup()
{
	size_t kip_size = &__kips_end - &__kips_start;
    LOGT("Kip blob size: 0x%X\n", kip_size);

    uintptr_t off = 0;
    while(off < kip_size)
    {
        KipHeader *hdr = (KipHeader*)(&__kips_start + off);
        LOGT("Module: %s\n", hdr->magic);
        ASSERT(kstrlen(hdr->magic) > 0);
        
        kProcessManager_CreateProcess(hdr->magic, (uint8_t*)hdr + hdr->headerSize, hdr->codeSize, 0);

        off += hdr->totalSize;
    }
}