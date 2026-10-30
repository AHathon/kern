#include "dwc2.h"
#include "libraries/services/svc.h"
#include "kernel/memory/kMemoryMap.h"
#include "libc/string.h"

#define REG(off)            (*(volatile uint32_t*)(USERLAND_USB_VIRT_BASE + (off)))

#define GAHBCFG             REG(0x008)
#define GUSBCFG             REG(0x00C)
#define GRSTCTL             REG(0x010)
#define GRXFSIZ             REG(0x024)
#define GNPTXFSIZ           REG(0x028)
#define GSNPSID             REG(0x040)
#define HPTXFSIZ            REG(0x100)
#define HFNUM               REG(0x408)
#define HPRT                REG(0x440)
#define PCGCCTL             REG(0xE00)
#define HCCHAR(c)           REG(0x500 + (c) * 0x20)
#define HCINT(c)            REG(0x508 + (c) * 0x20)
#define HCTSIZ(c)           REG(0x510 + (c) * 0x20)
#define HCDMA(c)            REG(0x514 + (c) * 0x20)

#define GSNPSID_MAGIC       0x4F54
#define GAHBCFG_DMA_EN      (1 << 5)
#define GUSBCFG_FORCE_HOST  (1 << 29)
#define GUSBCFG_FORCE_DEV   (1 << 30)
#define GRSTCTL_SOFT_RESET  (1 << 0)
#define GRSTCTL_RX_FLUSH    (1 << 4)
#define GRSTCTL_TX_FLUSH    (1 << 5)
#define GRSTCTL_TX_ALL      (0x10 << 6)
#define GRSTCTL_AHB_IDLE    (1UL << 31)

#define HPRT_CONNECTED      (1 << 0)
#define HPRT_CONN_CHANGE    (1 << 1)
#define HPRT_ENABLED        (1 << 2)
#define HPRT_EN_CHANGE      (1 << 3)
#define HPRT_OC_CHANGE      (1 << 5)
#define HPRT_RESET          (1 << 8)
#define HPRT_POWER          (1 << 12)
#define HPRT_SPEED(x)       (((x) >> 17) & 3)
#define HPRT_W1C           (HPRT_CONN_CHANGE | HPRT_ENABLED | HPRT_EN_CHANGE | HPRT_OC_CHANGE)

#define HCCHAR_MPS(x)       ((uint32_t)(x) << 0)
#define HCCHAR_EP(x)        ((uint32_t)(x) << 11)
#define HCCHAR_IN           (1 << 15)
#define HCCHAR_LOW_SPEED    (1 << 17)
#define HCCHAR_CONTROL      (0 << 18)
#define HCCHAR_INTERRUPT    (3 << 18)
#define HCCHAR_MC1          (1 << 20)
#define HCCHAR_ADDR(x)      ((uint32_t)(x) << 22)
#define HCCHAR_ODD_FRAME    (1 << 29)
#define HCCHAR_ENABLE       (1UL << 31)

#define HCINT_COMPLETE      (1 << 0)
#define HCINT_HALTED        (1 << 1)
#define HCINT_STALL         (1 << 3)
#define HCINT_NAK           (1 << 4)
#define HCINT_NYET          (1 << 6)

#define HCTSIZ_PACKETS(x)   ((uint32_t)(x) << 19)
#define HCTSIZ_DATA0        (0UL << 29)
#define HCTSIZ_DATA1        (2UL << 29)
#define HCTSIZ_SETUP        (3UL << 29)
#define HCTSIZ_SIZE_MASK    0x7FFFF

//The controller reaches RAM through the VC bus; this alias of it bypasses the VC cache
#define DMA_BUS_ALIAS       0xC0000000UL
#define DMA_BUS_BASE        (USERLAND_USB_DMA_PHYS | DMA_BUS_ALIAS)
#define DMA_SETUP_OFF       0x000
#define DMA_CONTROL_OFF     0x040
#define DMA_PIPE_OFF        0x800
#define DMA_PIPE_SIZE       0x40

#define CONTROL_CHANNEL     0
#define TIMEOUT_MS          100
#define MAX_NAK_RETRIES     50

static uint8_t *const dma = (uint8_t*)USERLAND_USB_DMA_VIRT_BASE;
static uint8_t pipeCount = 0;

static_assert(DMA_CONTROL_OFF + DWC2_MAX_CONTROL <= DMA_PIPE_OFF, "control buffer overlaps pipes");
static_assert(DMA_PIPE_OFF + DWC2_MAX_PIPES * DMA_PIPE_SIZE <= USERLAND_USB_DMA_SIZE, "pipes overrun the DMA region");

static uint64_t Counter()
{
    uint64_t v;
    asm volatile("isb; mrs %0, cntpct_el0" : "=r"(v));
    return v;
}

static uint64_t MsToTicks(uint32_t ms)
{
    uint64_t freq;
    asm volatile("mrs %0, cntfrq_el0" : "=r"(freq));
    return freq * ms / 1000;
}

void Dwc2_Delay(uint32_t ms)
{
    uint64_t start = Counter();
    uint64_t ticks = MsToTicks(ms);
    while(Counter() - start < ticks)
        svcYield();
}

static bool WaitBits(volatile uint32_t &reg, uint32_t mask, uint32_t value)
{
    uint64_t start = Counter();
    uint64_t ticks = MsToTicks(TIMEOUT_MS);
    while((reg & mask) != value)
        if(Counter() - start > ticks)
            return false;
    return true;
}

bool Dwc2_Init()
{
    if((GSNPSID >> 16) != GSNPSID_MAGIC)
        return false;

    if(!WaitBits(GRSTCTL, GRSTCTL_AHB_IDLE, GRSTCTL_AHB_IDLE))
        return false;
    GRSTCTL = GRSTCTL_SOFT_RESET;
    if(!WaitBits(GRSTCTL, GRSTCTL_SOFT_RESET, 0))
        return false;

    GUSBCFG = (GUSBCFG & ~GUSBCFG_FORCE_DEV) | GUSBCFG_FORCE_HOST;
    Dwc2_Delay(100);
    PCGCCTL = 0;

    GRXFSIZ = 0x306;
    GNPTXFSIZ = (0x100 << 16) | 0x306;
    HPTXFSIZ = (0x200 << 16) | 0x406;
    GRSTCTL = GRSTCTL_TX_FLUSH | GRSTCTL_TX_ALL;
    if(!WaitBits(GRSTCTL, GRSTCTL_TX_FLUSH, 0))
        return false;
    GRSTCTL = GRSTCTL_RX_FLUSH;
    if(!WaitBits(GRSTCTL, GRSTCTL_RX_FLUSH, 0))
        return false;

    GAHBCFG = GAHBCFG_DMA_EN;
    HPRT = (HPRT & ~HPRT_W1C) | HPRT_POWER;
    Dwc2_Delay(100);
    return true;
}

bool Dwc2_ResetPort(UsbSpeed *speed)
{
    if(!(HPRT & HPRT_CONNECTED))
        return false;

    HPRT = (HPRT & ~HPRT_W1C) | HPRT_RESET;
    Dwc2_Delay(60);
    HPRT = HPRT & ~(HPRT_W1C | HPRT_RESET);
    Dwc2_Delay(20);
    if(!(HPRT & HPRT_ENABLED))
        return false;

    *speed = (UsbSpeed)HPRT_SPEED(HPRT);
    return true;
}

static uint32_t EndpointBits(const UsbEndpoint *ep)
{
    uint32_t bits = HCCHAR_MPS(ep->maxPacket) | HCCHAR_EP(ep->endpoint) | HCCHAR_ADDR(ep->address) | HCCHAR_MC1;
    if(ep->speed == USB_SPEED_LOW)
        bits |= HCCHAR_LOW_SPEED;
    return bits;
}

static void StartChannel(uint8_t channel, uint32_t chr, uint32_t tsiz, uint32_t dmaOff)
{
    HCINT(channel) = 0xFFFFFFFF;
    HCDMA(channel) = DMA_BUS_BASE + dmaOff;
    HCTSIZ(channel) = tsiz;
    //Everything the transfer reads has to land in RAM before the channel starts
    asm volatile("dsb sy");
    HCCHAR(channel) = chr | HCCHAR_ENABLE;
}

static int ControlStage(uint32_t chr, uint32_t pid, uint32_t dmaOff, uint32_t size, uint32_t packets)
{
    for(int attempt = 0; attempt < MAX_NAK_RETRIES; attempt++)
    {
        StartChannel(CONTROL_CHANNEL, chr, pid | HCTSIZ_PACKETS(packets) | size, dmaOff);
        if(!WaitBits(HCINT(CONTROL_CHANNEL), HCINT_HALTED, HCINT_HALTED))
            return -1;

        uint32_t status = HCINT(CONTROL_CHANNEL);
        if(status & HCINT_COMPLETE)
            return size - (HCTSIZ(CONTROL_CHANNEL) & HCTSIZ_SIZE_MASK);
        if(!(status & (HCINT_NAK | HCINT_NYET)))
            return -1;
        Dwc2_Delay(1);
    }
    return -1;
}

int Dwc2_Control(const UsbEndpoint *ep, const UsbSetup *setup, void *data)
{
    size_t length = setup->length;
    if(length > DWC2_MAX_CONTROL)
        return -1;

    bool in = setup->requestType & 0x80;
    uint32_t chr = EndpointBits(ep) | HCCHAR_CONTROL;

    memcpy(dma + DMA_SETUP_OFF, setup, sizeof(UsbSetup));
    if(ControlStage(chr, HCTSIZ_SETUP, DMA_SETUP_OFF, sizeof(UsbSetup), 1) < 0)
        return -1;

    int transferred = 0;
    if(length)
    {
        //Whole packets, so a device with more to send can't babble past the request
        uint32_t packets = (length + ep->maxPacket - 1) / ep->maxPacket;
        uint32_t size = in ? packets * ep->maxPacket : length;
        if(size > DWC2_MAX_CONTROL)
            return -1;
        if(!in)
            memcpy(dma + DMA_CONTROL_OFF, data, length);

        transferred = ControlStage(chr | (in ? HCCHAR_IN : 0), HCTSIZ_DATA1, DMA_CONTROL_OFF, size, packets);
        if(transferred < 0)
            return -1;
        if(in)
        {
            asm volatile("dsb sy");
            if((size_t)transferred > length)
                transferred = length;
            memcpy(data, dma + DMA_CONTROL_OFF, transferred);
        }
    }

    uint32_t statusDir = (in && length) ? 0 : HCCHAR_IN;
    if(ControlStage(chr | statusDir, HCTSIZ_DATA1, DMA_CONTROL_OFF, 0, 1) < 0)
        return -1;
    return transferred;
}

static void StartPipe(Dwc2Pipe *pipe)
{
    uint32_t chr = EndpointBits(&pipe->ep) | HCCHAR_INTERRUPT | HCCHAR_IN;
    if(!(HFNUM & 1))
        chr |= HCCHAR_ODD_FRAME;

    uint32_t pid = pipe->toggle ? HCTSIZ_DATA1 : HCTSIZ_DATA0;
    uint32_t dmaOff = DMA_PIPE_OFF + (pipe->channel - 1) * DMA_PIPE_SIZE;
    StartChannel(pipe->channel, chr, pid | HCTSIZ_PACKETS(1) | pipe->ep.maxPacket, dmaOff);
}

bool Dwc2_OpenPipe(Dwc2Pipe *pipe, const UsbEndpoint *ep)
{
    if(pipeCount >= DWC2_MAX_PIPES || ep->maxPacket == 0 || ep->maxPacket > DMA_PIPE_SIZE)
        return false;

    pipe->ep = *ep;
    pipe->channel = ++pipeCount;
    pipe->toggle = 0;
    StartPipe(pipe);
    return true;
}

int Dwc2_PollPipe(Dwc2Pipe *pipe, void *data, size_t size)
{
    uint32_t status = HCINT(pipe->channel);
    if(!(status & HCINT_HALTED))
        return 0;

    int received = 0;
    if(status & HCINT_COMPLETE)
    {
        pipe->toggle ^= 1;
        asm volatile("dsb sy");
        received = pipe->ep.maxPacket - (HCTSIZ(pipe->channel) & HCTSIZ_SIZE_MASK);
        if((size_t)received > size)
            received = size;
        memcpy(data, dma + DMA_PIPE_OFF + (pipe->channel - 1) * DMA_PIPE_SIZE, received);
    }
    else if(status & HCINT_STALL)
        return -1;

    StartPipe(pipe);
    return received;
}
