#include "libraries/hardware/mailbox.h"
#include "libraries/hardware/mmio.h"
#include "libraries/hardware/mmio_vars.h"

volatile uint32_t __attribute__((aligned(16))) mbox[36];

uint32_t Mailbox_Call(uint8_t c)
{
    //[31:4] = address of message buffer
    //[3:0] = channel number
    uint32_t r = ((uint32_t)((uintptr_t)&mbox) &~ 0xF) | (c & 0xF);

    //Wait until we can write
    while (*VC_MBOX_STATUS & MBOX_FULL);
    
    //Write the address of our buffer to the mailbox with the channel appended
    *VC_MBOX_WRITE = r;

    while (1) {
        while (*VC_MBOX_STATUS & MBOX_EMPTY);

        uint32_t response = *VC_MBOX_READ;
        if ((response & ~0xF) == ((uint32_t)((long)&mbox) & ~0xF))
            return mbox[1] == MBOX_RESPONSE;
    }

    return 0;
}