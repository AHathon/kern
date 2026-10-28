#include "libraries/hardware/emmc.h"
#include "libraries/hardware/mmio_vars.h"
#include "libraries/hardware/debug.h"
#include "kernel/memory/kMemoryMap.h"
#include "libraries/types.h"

static uint8_t sector[EMMC_BLOCK_SIZE];

int main() 
{
    //KIPs run at EL1 but link their own copy of the libraries; point them at the kernel's linear map
    MMIO_ADDR = MMIO_BASE + KERNEL_VIRT_BASE;

    if(EMMC_Init() != 0)
    {
        ERROR("[fs] SD init failed\n");
        return 0;
    }

    //Sanity check: MBR signature and first partition
    if(EMMC_ReadBlocks(0, 1, sector) != 0 || sector[510] != 0x55 || sector[511] != 0xAA)
    {
        ERROR("[fs] no valid MBR on SD\n");
        return 0;
    }

    uint8_t *part = &sector[446];
    uint32_t lba = part[8] | (part[9] << 8) | (part[10] << 16) | (part[11] << 24);
    uint32_t cnt = part[12] | (part[13] << 8) | (part[14] << 16) | (part[15] << 24);
    LOG("[fs] partition 0: type 0x%X @ lba 0x%X [0x%X sectors]\n", part[4], lba, cnt);

    return 0;
}