#include "libraries/hardware/emmc.h"
#include "libraries/hardware/mmio_vars.h"
#include "libraries/hardware/debug.h"

//Controller register offsets (BCM EMMC names for the standard SDHCI layout)
#define EMMC_BLKSIZECNT     0x04
#define EMMC_ARG1           0x08
#define EMMC_CMDTM          0x0C
#define EMMC_RESP0          0x10
#define EMMC_DATA           0x20
#define EMMC_STATUS         0x24
#define EMMC_CONTROL0       0x28
#define EMMC_CONTROL1       0x2C
#define EMMC_INTERRUPT      0x30
#define EMMC_IRPT_MASK      0x34
#define EMMC_IRPT_EN        0x38
#define EMMC_CONTROL2       0x3C
#define EMMC_CAPS0          0x40

//STATUS
#define STATUS_CMD_INHIBIT  (1 << 0)
#define STATUS_DAT_INHIBIT  (1 << 1)
#define STATUS_CARD_INSERTED (1 << 16)

//CONTROL0
#define C0_POWER_3V3        (0xF << 8)  //bus power on, 3.3V

//CONTROL1
#define C1_CLK_INTLEN       (1 << 0)
#define C1_CLK_STABLE       (1 << 1)
#define C1_CLK_EN           (1 << 2)
#define C1_DATA_TOUNIT_MAX  (0xE << 16)
#define C1_SRST_HC          (1 << 24)
#define C1_SRST_CMD         (1 << 25)
#define C1_SRST_DATA        (1 << 26)

//INTERRUPT
#define INT_CMD_DONE        (1 << 0)
#define INT_DATA_DONE       (1 << 1)
#define INT_READ_RDY        (1 << 5)
#define INT_ERR             (1 << 15)

//CMDTM
#define TM_BLKCNT_EN        (1 << 1)
#define TM_DAT_DIR_READ     (1 << 4)
#define CMD_RSPNS_NONE      (0 << 16)
#define CMD_RSPNS_136       (1 << 16)
#define CMD_RSPNS_48        (2 << 16)
#define CMD_RSPNS_48_BUSY   (3 << 16)
#define CMD_CRCCHK_EN       (1 << 19)
#define CMD_IXCHK_EN        (1 << 20)
#define CMD_ISDATA          (1 << 21)
#define CMD_INDEX(n)        ((n) << 24)

#define RESP_R1     (CMD_RSPNS_48 | CMD_CRCCHK_EN | CMD_IXCHK_EN)
#define RESP_R1b    (CMD_RSPNS_48_BUSY | CMD_CRCCHK_EN | CMD_IXCHK_EN)
#define RESP_R2     (CMD_RSPNS_136 | CMD_CRCCHK_EN)
#define RESP_R3     (CMD_RSPNS_48)
#define RESP_R6     RESP_R1
#define RESP_R7     RESP_R1

#define SD_CMD0     (CMD_INDEX(0)  | CMD_RSPNS_NONE)                 //GO_IDLE_STATE
#define SD_CMD2     (CMD_INDEX(2)  | RESP_R2)                        //ALL_SEND_CID
#define SD_CMD3     (CMD_INDEX(3)  | RESP_R6)                        //SEND_RELATIVE_ADDR
#define SD_CMD7     (CMD_INDEX(7)  | RESP_R1b)                       //SELECT_CARD
#define SD_CMD8     (CMD_INDEX(8)  | RESP_R7)                        //SEND_IF_COND
#define SD_CMD16    (CMD_INDEX(16) | RESP_R1)                        //SET_BLOCKLEN
#define SD_CMD17    (CMD_INDEX(17) | RESP_R1 | CMD_ISDATA | TM_DAT_DIR_READ | TM_BLKCNT_EN) //READ_SINGLE_BLOCK
#define SD_CMD55    (CMD_INDEX(55) | RESP_R1)                        //APP_CMD
#define SD_ACMD41   (CMD_INDEX(41) | RESP_R3)                        //SD_SEND_OP_COND

#define OCR_BUSY    (1u << 31)  //set when card finished power up
#define OCR_CCS     (1u << 30)  //SDHC/SDXC (block addressed)
#define OCR_HCS     (1u << 30)
#define OCR_3V3     (0x00FF8000)

static uintptr_t emmcBase = 0;
static uint8_t isBlockAddressed = 0;

static inline volatile uint32_t *EMMC_Reg(uint32_t off)
{
    return (volatile uint32_t *)(emmcBase + off);
}

static uint64_t EMMC_TimeUs()
{
    uint64_t cnt, freq;
    asm volatile("isb; mrs %0, cntpct_el0" : "=r"(cnt));
    asm volatile("mrs %0, cntfrq_el0" : "=r"(freq));
    return cnt / (freq / 1000000);
}

static void EMMC_DelayUs(uint64_t us)
{
    uint64_t end = EMMC_TimeUs() + us;
    while(EMMC_TimeUs() < end);
}

//Wait until (reg & mask) != 0 (or == 0 if waitClear); returns 0 on timeout
static int EMMC_Wait(uint32_t off, uint32_t mask, uint8_t waitClear, uint64_t timeoutUs)
{
    uint64_t end = EMMC_TimeUs() + timeoutUs;
    while(EMMC_TimeUs() < end)
    {
        uint32_t v = *EMMC_Reg(off) & mask;
        if(waitClear ? (v == 0) : (v != 0))
            return 1;
    }
    return 0;
}

static int EMMC_ResetLines(uint32_t lines)
{
    *EMMC_Reg(EMMC_CONTROL1) |= lines;
    return EMMC_Wait(EMMC_CONTROL1, lines, 1, 100000);
}

static int EMMC_Cmd(uint32_t cmdtm, uint32_t arg, uint32_t *resp)
{
    if(!EMMC_Wait(EMMC_STATUS, STATUS_CMD_INHIBIT, 1, 100000))
        return -1;
    if((cmdtm & CMD_ISDATA) || (cmdtm & CMD_RSPNS_48_BUSY) == CMD_RSPNS_48_BUSY)
        if(!EMMC_Wait(EMMC_STATUS, STATUS_DAT_INHIBIT, 1, 500000))
            return -1;

    *EMMC_Reg(EMMC_INTERRUPT) = 0xFFFFFFFF;
    *EMMC_Reg(EMMC_ARG1) = arg;
    *EMMC_Reg(EMMC_CMDTM) = cmdtm;

    if(!EMMC_Wait(EMMC_INTERRUPT, INT_CMD_DONE | INT_ERR, 0, 100000))
    {
        EMMC_ResetLines(C1_SRST_CMD);
        return -1;
    }

    uint32_t irq = *EMMC_Reg(EMMC_INTERRUPT);
    *EMMC_Reg(EMMC_INTERRUPT) = INT_CMD_DONE | INT_ERR | 0xFFFF0000;
    if(irq & INT_ERR)
    {
        EMMC_ResetLines(C1_SRST_CMD);
        return -1;
    }

    if(resp)
        for(int i = 0; i < 4; i++)
            resp[i] = *EMMC_Reg(EMMC_RESP0 + i * 4);

    //R1b: card signals busy on DAT0 until it's done
    if((cmdtm & CMD_RSPNS_48_BUSY) == CMD_RSPNS_48_BUSY)
        EMMC_Wait(EMMC_STATUS, STATUS_DAT_INHIBIT, 1, 500000);

    return 0;
}

static int EMMC_AppCmd(uint32_t cmdtm, uint32_t arg, uint32_t rca, uint32_t *resp)
{
    if(EMMC_Cmd(SD_CMD55, rca << 16, 0) != 0)
        return -1;
    return EMMC_Cmd(cmdtm, arg, resp);
}

static int EMMC_SetClock(uint32_t hz)
{
    //Base clock (MHz) from capabilities; 0 means "get it elsewhere"
    uint32_t baseHz = ((*EMMC_Reg(EMMC_CAPS0) >> 8) & 0xFF) * 1000000;
    if(baseHz == 0)
        baseHz = 100000000; //Pi4 EMMC2 default from firmware

    //SDHCI v3 10-bit divider: f = base / (2 * div), div 0 = base
    uint32_t div = 0;
    if(hz < baseHz)
    {
        div = (baseHz + 2 * hz - 1) / (2 * hz);
        if(div > 0x3FF)
            div = 0x3FF;
    }

    if(!EMMC_Wait(EMMC_STATUS, STATUS_CMD_INHIBIT | STATUS_DAT_INHIBIT, 1, 100000))
        return -1;

    uint32_t c1 = *EMMC_Reg(EMMC_CONTROL1);
    c1 &= ~(C1_CLK_EN | 0xFFC0);
    *EMMC_Reg(EMMC_CONTROL1) = c1;

    c1 |= ((div & 0xFF) << 8) | (((div >> 8) & 0x3) << 6) | C1_CLK_INTLEN;
    *EMMC_Reg(EMMC_CONTROL1) = c1;
    if(!EMMC_Wait(EMMC_CONTROL1, C1_CLK_STABLE, 0, 100000))
        return -1;

    *EMMC_Reg(EMMC_CONTROL1) = c1 | C1_CLK_EN;
    EMMC_DelayUs(2000);
    return 0;
}

//Reset the controller at base; returns 1 if it has a card in the slot
static int EMMC_ProbeController(uintptr_t base)
{
    emmcBase = base;
    *EMMC_Reg(EMMC_CONTROL2) = 0;
    if(!EMMC_ResetLines(C1_SRST_HC))
        return 0;
    return (*EMMC_Reg(EMMC_STATUS) & STATUS_CARD_INSERTED) != 0;
}

int EMMC_Init()
{
    //Pi4 boots from EMMC2; QEMU (and Pi3) attach the card to the legacy EMMC controller
#ifndef PI3
    if(!EMMC_ProbeController(MMIO_ADDR + 0x340000))
#endif
    if(!EMMC_ProbeController(MMIO_ADDR + 0x300000))
    {
        ERROR("[emmc] no card found\n");
        return -1;
    }
    LOG("[emmc] card found on controller @ 0x%X\n", emmcBase);

    *EMMC_Reg(EMMC_CONTROL0) = C0_POWER_3V3;
    *EMMC_Reg(EMMC_CONTROL1) = C1_DATA_TOUNIT_MAX;
    if(EMMC_SetClock(400000) != 0)
    {
        ERROR("[emmc] clock didn't stabilize\n");
        return -1;
    }

    //Poll status bits, no interrupt lines
    *EMMC_Reg(EMMC_IRPT_EN) = 0;
    *EMMC_Reg(EMMC_IRPT_MASK) = 0xFFFFFFFF;
    *EMMC_Reg(EMMC_INTERRUPT) = 0xFFFFFFFF;

    uint32_t resp[4];
    if(EMMC_Cmd(SD_CMD0, 0, 0) != 0)
    {
        ERROR("[emmc] CMD0 failed\n");
        return -1;
    }

    //CMD8 only answers on v2+ cards (needed for SDHC)
    uint8_t isV2 = EMMC_Cmd(SD_CMD8, 0x1AA, resp) == 0 && (resp[0] & 0xFFF) == 0x1AA;

    uint32_t ocr = 0;
    uint64_t end = EMMC_TimeUs() + 1000000;
    do
    {
        if(EMMC_AppCmd(SD_ACMD41, OCR_3V3 | (isV2 ? OCR_HCS : 0), 0, resp) != 0)
        {
            ERROR("[emmc] ACMD41 failed\n");
            return -1;
        }
        ocr = resp[0];
        if(!(ocr & OCR_BUSY))
            EMMC_DelayUs(10000);
    } while(!(ocr & OCR_BUSY) && EMMC_TimeUs() < end);

    if(!(ocr & OCR_BUSY))
    {
        ERROR("[emmc] card never left busy\n");
        return -1;
    }
    isBlockAddressed = (ocr & OCR_CCS) != 0;

    if(EMMC_Cmd(SD_CMD2, 0, resp) != 0 || EMMC_Cmd(SD_CMD3, 0, resp) != 0)
    {
        ERROR("[emmc] card identification failed\n");
        return -1;
    }
    uint32_t rca = resp[0] >> 16;

    if(EMMC_SetClock(25000000) != 0 || EMMC_Cmd(SD_CMD7, rca << 16, 0) != 0)
    {
        ERROR("[emmc] card select failed\n");
        return -1;
    }

    if(!isBlockAddressed && EMMC_Cmd(SD_CMD16, EMMC_BLOCK_SIZE, 0) != 0)
    {
        ERROR("[emmc] set block length failed\n");
        return -1;
    }

    LOG("[emmc] %s card ready [rca:0x%X]\n", isBlockAddressed ? "SDHC/SDXC" : "SDSC", rca);
    return 0;
}

int EMMC_ReadBlocks(uint32_t lba, uint32_t count, uint8_t *buf)
{
    if(!emmcBase)
        return -1;

    for(uint32_t b = 0; b < count; b++)
    {
        uint32_t addr = isBlockAddressed ? (lba + b) : (lba + b) * EMMC_BLOCK_SIZE;

        *EMMC_Reg(EMMC_BLKSIZECNT) = (1 << 16) | EMMC_BLOCK_SIZE;
        if(EMMC_Cmd(SD_CMD17, addr, 0) != 0)
            return -1;

        if(!EMMC_Wait(EMMC_INTERRUPT, INT_READ_RDY | INT_ERR, 0, 500000) ||
            (*EMMC_Reg(EMMC_INTERRUPT) & INT_ERR))
        {
            EMMC_ResetLines(C1_SRST_DATA);
            return -1;
        }
        *EMMC_Reg(EMMC_INTERRUPT) = INT_READ_RDY;

        uint8_t *dst = buf + b * EMMC_BLOCK_SIZE;
        for(int i = 0; i < EMMC_BLOCK_SIZE / 4; i++)
        {
            uint32_t w = *EMMC_Reg(EMMC_DATA);
            dst[i * 4 + 0] = w;
            dst[i * 4 + 1] = w >> 8;
            dst[i * 4 + 2] = w >> 16;
            dst[i * 4 + 3] = w >> 24;
        }

        if(!EMMC_Wait(EMMC_INTERRUPT, INT_DATA_DONE | INT_ERR, 0, 500000) ||
            (*EMMC_Reg(EMMC_INTERRUPT) & INT_ERR))
        {
            EMMC_ResetLines(C1_SRST_DATA);
            return -1;
        }
        *EMMC_Reg(EMMC_INTERRUPT) = INT_DATA_DONE;
    }

    return 0;
}
