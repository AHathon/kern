#include "libraries/hardware/pcie.h"
#include "libraries/hardware/constants.h"
#include "libraries/hardware/debug.h"
#include "libraries/hardware/mailbox.h"
#include "libraries/hardware/mmio_vars.h"
#include "libraries/libc/time.h"


#define VENDOR_REG1             0x0188
#define VENDOR_REG1_ENDIAN_BAR2 (3 << 2)
#define PRIV1_ID_VAL3           0x043C
#define PRIV1_CLASS_MASK        0xFFFFFF
#define PRIV1_LINK_CAP          0x04DC
#define PRIV1_ASPM_MASK         (3 << 10)

#define MISC_CTRL               0x4008
#define MISC_CTRL_SCB_ACCESS_EN (1 << 12)
#define MISC_CTRL_CFG_UR_MODE   (1 << 13)
#define MISC_CTRL_BURST_MASK    (3 << 20)
#define MISC_CTRL_SCB0_MASK     (0x1FUL << 27)
#define MISC_CTRL_SCB0_SHIFT    27

#define WIN0_LO                 0x400C
#define WIN0_HI                 0x4010
#define WIN0_BASE_LIMIT         0x4070
#define WIN0_BASE_HI            0x4080
#define WIN0_LIMIT_HI           0x4084

#define RC_BAR1_LO              0x402C
#define RC_BAR2_LO              0x4034
#define RC_BAR2_HI              0x4038
#define RC_BAR3_LO              0x403C
#define RC_BAR_SIZE_MASK        0x1F
#define UBUS_BAR2_REMAP         0x40B4
#define UBUS_BAR2_REMAP_ACCESS  (1 << 0)

#define HARD_DEBUG              0x4204
#define HARD_DEBUG_CLKREQ       (1 << 1)
#define HARD_DEBUG_SERDES_IDDQ  (1 << 27)

#define MISC_STATUS             0x4068
#define MISC_STATUS_RC_MODE     (1 << 7)
#define MISC_STATUS_LINK_UP     0x30
#define MISC_REVISION           0x406C

#define CFG_DATA                0x8000
#define CFG_INDEX               0x9000
#define CFG_BUS_SHIFT           20

#define SW_INIT                 0x9210
#define SW_INIT_PERST           (1 << 0)
#define SW_INIT_GENERIC         (1 << 1)

#define CFG_COMMAND             0x04
#define CFG_CLASS_REVISION      0x08
#define CFG_CACHE_LINE          0x0C
#define CFG_BAR0                0x10
#define CFG_BAR1                0x14
#define CFG_SECONDARY_BUS       0x19
#define CFG_SUBORDINATE_BUS     0x1A
#define CFG_MEMORY_BASE         0x20
#define CFG_MEMORY_LIMIT        0x22
#define CFG_BRIDGE_CONTROL      0x3E
#define CFG_ROOT_CONTROL        0xC8

#define COMMAND_MEMORY          (1 << 1)
#define COMMAND_MASTER          (1 << 2)
#define COMMAND_PARITY          (1 << 6)
#define COMMAND_SERR            (1 << 8)
#define BRIDGE_CONTROL_PARITY   (1 << 0)
#define ROOT_CONTROL_CRS        (1 << 4)
#define BAR_MEM_64              (1 << 2)

#define CLASS_BRIDGE            0x060400
#define CLASS_XHCI              0x0C0330
#define CACHE_LINE_DWORDS       (64 / 4)

#define WINDOW_PCI_ADDR         0xF8000000UL
#define WINDOW_SIZE             0x04000000ULL

//Inbound is 1:1, so PCI bus addresses are physical addresses
#define INBOUND_LOG2_SIZE       32
#define IBAR_SIZE_ENCODING      (INBOUND_LOG2_SIZE - 15)
#define SCB_SIZE_ENCODING       (INBOUND_LOG2_SIZE - 15)

#define XHCI_DEVICE_ADDR        (1UL << CFG_BUS_SHIFT)
#define MIN_FIRMWARE_REVISION   0x40000000 //a build timestamp, on real firmware
#define PERST_SETTLE_MS         100
#define LINK_TIMEOUT_MS         100

static uintptr_t regs;

static uint32_t rd32(uint32_t off)
{
    return *(volatile uint32_t*)(regs + off);
}

static void wr32(uint32_t off, uint32_t value)
{
    *(volatile uint32_t*)(regs + off) = value;
}

static void wr16(uint32_t off, uint16_t value)
{
    *(volatile uint16_t*)(regs + off) = value;
}

static void wr8(uint32_t off, uint8_t value)
{
    *(volatile uint8_t*)(regs + off) = value;
}

static void Clear(uint32_t off, uint32_t bits)
{
    wr32(off, rd32(off) & ~bits);
}

static void Set(uint32_t off, uint32_t bits)
{
    wr32(off, rd32(off) | bits);
}

static void DelayUs(uint64_t us)
{
    uint64_t start = Counter();
    uint64_t ticks = CounterFreq() * us / 1000000;
    while(Counter() - start < ticks);
}

static int LinkUp()
{
    return (rd32(MISC_STATUS) & MISC_STATUS_LINK_UP) == MISC_STATUS_LINK_UP;
}

//QEMU's raspi4b has nothing at PCIE_BASE and any access aborts, so it has to be ruled out before touching it
static int HasController()
{
    mbox[0] = 7 * 4;
    mbox[1] = MBOX_REQUEST;
    mbox[2] = TAG_GET_FW_REVISION;
    mbox[3] = 4;
    mbox[4] = 0;
    mbox[5] = 0;
    mbox[6] = TAG_LAST;
    return Mailbox_Call(CH_PROP) && mbox[5] >= MIN_FIRMWARE_REVISION;
}

static void ResetBridge()
{
    Set(SW_INIT, SW_INIT_GENERIC);
    Set(SW_INIT, SW_INIT_PERST);
    DelayUs(100);
    Clear(SW_INIT, SW_INIT_GENERIC);

    Clear(HARD_DEBUG, HARD_DEBUG_SERDES_IDDQ);
    DelayUs(100);
}

static void SetupInbound()
{
    uint32_t ctrl = rd32(MISC_CTRL) | MISC_CTRL_SCB_ACCESS_EN | MISC_CTRL_CFG_UR_MODE;
    ctrl &= ~MISC_CTRL_BURST_MASK; //128 bytes
    ctrl = (ctrl & ~MISC_CTRL_SCB0_MASK) | ((uint32_t)SCB_SIZE_ENCODING << MISC_CTRL_SCB0_SHIFT);
    wr32(MISC_CTRL, ctrl);

    wr32(RC_BAR2_LO, IBAR_SIZE_ENCODING);
    wr32(RC_BAR2_HI, 0);
    Set(UBUS_BAR2_REMAP, UBUS_BAR2_REMAP_ACCESS);

    Clear(RC_BAR1_LO, RC_BAR_SIZE_MASK);
    Clear(RC_BAR3_LO, RC_BAR_SIZE_MASK);
    Set(PRIV1_LINK_CAP, PRIV1_ASPM_MASK);
}

static void SetupOutbound()
{
    uint64_t baseMb = PCIE_XHCI_BASE >> 20;
    uint64_t limitMb = (PCIE_XHCI_BASE + WINDOW_SIZE - 1) >> 20;

    wr32(WIN0_LO, WINDOW_PCI_ADDR);
    wr32(WIN0_HI, 0);
    wr32(WIN0_BASE_LIMIT, ((baseMb & 0xFFF) << 4) | ((limitMb & 0xFFF) << 20));
    wr32(WIN0_BASE_HI, baseMb >> 12);
    wr32(WIN0_LIMIT_HI, limitMb >> 12);
}

static int WaitForLink()
{
    Clear(SW_INIT, SW_INIT_PERST);
    DelayUs(PERST_SETTLE_MS * 1000);

    for(int ms = 0; ms < LINK_TIMEOUT_MS && !LinkUp(); ms++)
        DelayUs(1000);
    return LinkUp();
}

static int EnableBridge()
{
    if(rd32(CFG_CLASS_REVISION) >> 8 != CLASS_BRIDGE)
        return 0;

    wr8(CFG_CACHE_LINE, CACHE_LINE_DWORDS);
    wr8(CFG_SECONDARY_BUS, 1);
    wr8(CFG_SUBORDINATE_BUS, 1);
    wr16(CFG_MEMORY_BASE, WINDOW_PCI_ADDR >> 16);
    wr16(CFG_MEMORY_LIMIT, WINDOW_PCI_ADDR >> 16);
    wr8(CFG_BRIDGE_CONTROL, BRIDGE_CONTROL_PARITY);
    wr8(CFG_ROOT_CONTROL, ROOT_CONTROL_CRS);
    wr16(CFG_COMMAND, COMMAND_MEMORY | COMMAND_MASTER | COMMAND_PARITY | COMMAND_SERR);
    return 1;
}

//VideoCore loads the VL805's firmware if the board has no EEPROM for it
static int NotifyXhciReset()
{
    mbox[0] = 7 * 4;
    mbox[1] = MBOX_REQUEST;
    mbox[2] = TAG_NOTIFY_XHCI_RESET;
    mbox[3] = 4;
    mbox[4] = 4;
    mbox[5] = XHCI_DEVICE_ADDR;
    mbox[6] = TAG_LAST;
    if(!Mailbox_Call(CH_PROP))
        return 0;

    DelayUs(1000);
    return 1;
}

static int EnableXhci()
{
    wr32(CFG_INDEX, XHCI_DEVICE_ADDR);
    uint32_t class = rd32(CFG_DATA + CFG_CLASS_REVISION) >> 8;
    if(class != CLASS_XHCI)
    {
        ERROR("[pcie] bus 1 device class is 0x%X, not xHCI\n", class);
        return 0;
    }

    wr8(CFG_DATA + CFG_CACHE_LINE, CACHE_LINE_DWORDS);
    wr32(CFG_DATA + CFG_BAR0, WINDOW_PCI_ADDR | BAR_MEM_64);
    wr32(CFG_DATA + CFG_BAR1, 0);
    wr16(CFG_DATA + CFG_COMMAND, COMMAND_MEMORY | COMMAND_MASTER | COMMAND_PARITY | COMMAND_SERR);
    return 1;
}

int Pcie_Init()
{
    if(!HasController())
    {
        LOG("[pcie] no controller [fw:0x%X]\n", mbox[5]);
        return 0;
    }

    regs = PCIE_BASE + (MMIO_ADDR - MMIO_BASE);
    ResetBridge();
    if(!(rd32(MISC_STATUS) & MISC_STATUS_RC_MODE))
    {
        ERROR("[pcie] controller is in endpoint mode\n");
        return 0;
    }

    SetupInbound();
    if(!WaitForLink())
    {
        ERROR("[pcie] link down\n");
        return 0;
    }
    LOG("[pcie] link up [rev:0x%X]\n", rd32(MISC_REVISION));

    SetupOutbound();
    wr32(PRIV1_ID_VAL3, (rd32(PRIV1_ID_VAL3) & ~PRIV1_CLASS_MASK) | CLASS_BRIDGE);
    Clear(VENDOR_REG1, VENDOR_REG1_ENDIAN_BAR2);
    Set(HARD_DEBUG, HARD_DEBUG_CLKREQ);

    if(EnableBridge() && NotifyXhciReset() && EnableXhci())
        return 1;

    ERROR("[pcie] couldn't enable the xHCI controller\n");
    return 0;
}
