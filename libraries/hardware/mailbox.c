#include "libraries/hardware/mailbox.h"
#include "libraries/hardware/mmio.h"
#include "libraries/hardware/mmio_vars.h"

volatile uint32_t __attribute__((aligned(16))) mbox[36];

static uint32_t Mailbox_GetBufferPhys()
{
    uintptr_t va = (uintptr_t)&mbox;
    uint64_t par;
    asm volatile("at s1e1r, %0" : : "r"(va));
    asm volatile("isb");
    asm volatile("mrs %0, par_el1" : "=r"(par));
    return (uint32_t)((par & 0xFFFFFFFFF000ULL) | (va & 0xFFF));
}

uint32_t Mailbox_Call(uint8_t c)
{
    uint32_t mboxPhys = Mailbox_GetBufferPhys();

    //[31:4] = address of message buffer
    //[3:0] = channel number
    uint32_t r = (mboxPhys &~ 0xF) | (c & 0xF);

    while (*VC_MBOX_STATUS & MBOX_FULL);
    
    *VC_MBOX_WRITE = r;

    while (1) {
        while (*VC_MBOX_STATUS & MBOX_EMPTY);

        uint32_t response = *VC_MBOX_READ;
        if ((response & ~0xF) == (mboxPhys & ~0xF))
            return mbox[1] == MBOX_RESPONSE;
    }

    return 0;
}