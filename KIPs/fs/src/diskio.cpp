#include "fatfs/ff.h"
#include "fatfs/diskio.h"
#include "libraries/hardware/emmc.h"

static DSTATUS status = STA_NOINIT;

DSTATUS disk_status(BYTE pdrv)
{
    return pdrv == 0 ? status : STA_NOINIT;
}

//Runs once per mounted volume, but they all share the one card
DSTATUS disk_initialize(BYTE pdrv)
{
    if(pdrv == 0 && (status & STA_NOINIT) && EMMC_Init() == 0)
        status = 0;
    return disk_status(pdrv);
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count)
{
    if(disk_status(pdrv) & STA_NOINIT)
        return RES_NOTRDY;
    return EMMC_ReadBlocks(sector, count, buff) == 0 ? RES_OK : RES_ERROR;
}
