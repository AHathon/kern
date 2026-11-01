#include "driver/xhci.h"
#include "libraries/services/svc.h"
#include "libraries/libc/stdio.h"
#include "libraries/libc/string.h"
#include "libraries/libc/time.h"

#define CAP_LENGTH          0x00
#define CAP_HCSPARAMS1      0x04
#define CAP_HCSPARAMS2      0x08
#define CAP_HCCPARAMS1      0x10
#define CAP_DBOFF           0x14
#define CAP_RTSOFF          0x18
#define HCS1_SLOTS(x)       ((x) & 0xFF)
#define HCS1_PORTS(x)       ((x) >> 24)
#define HCS2_SCRATCH(x)     ((((x) >> 16) & 0x3E0) | ((x) >> 27))
#define HCC1_CONTEXT_64     (1 << 2)

#define OP_USBCMD           0x00
#define OP_USBSTS           0x04
#define OP_CRCR             0x18
#define OP_DCBAAP           0x30
#define OP_CONFIG           0x38
#define OP_PORTSC(p)        (0x400 + ((p) - 1) * 0x10)
#define USBCMD_RUN          (1 << 0)
#define USBCMD_RESET        (1 << 1)
#define USBSTS_HALTED       (1 << 0)
#define USBSTS_NOT_READY    (1 << 11)

#define PORTSC_CONNECTED    (1 << 0)
#define PORTSC_ENABLED      (1 << 1)
#define PORTSC_RESET        (1 << 4)
#define PORTSC_POWER        (1 << 9)
#define PORTSC_SPEED(x)     (((x) >> 10) & 0xF)
#define PORTSC_RESET_CHANGE (1 << 21)
#define PORTSC_CHANGES      (0x7F << 17)
#define PORTSC_PRESERVE     0x0E00C3E0 //everything that isn't write-1-to-clear or an action

#define RT_MFINDEX          0x00
#define RT_ERSTSZ           0x28
#define RT_ERSTBA           0x30
#define RT_ERDP             0x38
#define ERDP_BUSY           (1 << 3)

#define SPEED_FULL          1
#define SPEED_LOW           2
#define SPEED_HIGH          3

#define TRB_CYCLE           (1 << 0)
#define TRB_TOGGLE_CYCLE    (1 << 1)
#define TRB_SHORT_INTERRUPT (1 << 2)
#define TRB_IOC             (1 << 5)
#define TRB_IMMEDIATE       (1 << 6)
#define TRB_TYPE(t)         ((uint32_t)(t) << 10)
#define TRB_GET_TYPE(x)     (((x) >> 10) & 0x3F)
#define TRB_SLOT(s)         ((uint32_t)(s) << 24)
#define TRB_ENDPOINT(e)     ((uint32_t)(e) << 16)
#define TRB_DIR_IN          (1 << 16)
#define TRB_SETUP_IN        (3 << 16)
#define TRB_SETUP_OUT       (2 << 16)

#define TYPE_NORMAL         1
#define TYPE_SETUP          2
#define TYPE_DATA           3
#define TYPE_STATUS         4
#define TYPE_LINK           6
#define TYPE_ENABLE_SLOT    9
#define TYPE_ADDRESS_DEVICE 11
#define TYPE_CONFIGURE_EP   12
#define TYPE_EVALUATE       13
#define TYPE_RESET_EP       14
#define TYPE_SET_DEQUEUE    16
#define TYPE_TRANSFER_EVENT 32
#define TYPE_COMMAND_EVENT  33

#define CODE_SUCCESS        1
#define CODE_SHORT_PACKET   13
#define CODE_MISSED_SERVICE 23

#define SLOT_ROUTE_MASK     0xFFFFF
#define SLOT_SPEED(s)       ((uint32_t)(s) << 20)
#define SLOT_HUB            (1 << 26)
#define SLOT_ENTRIES(n)     ((uint32_t)(n) << 27)
#define SLOT_ENTRIES_MASK   (0x1FUL << 27)
#define SLOT_GET_ENTRIES(x) ((x) >> 27)
#define SLOT_ROOT_PORT(p)   ((uint32_t)(p) << 16)
#define SLOT_PORTS(n)       ((uint32_t)(n) << 24)
#define SLOT_TT_HUB(s)      ((uint32_t)(s) << 0)
#define SLOT_TT_PORT(p)     ((uint32_t)(p) << 8)
#define SLOT_TT_THINK(t)    ((uint32_t)(t) << 16)

#define EP_INTERVAL(i)      ((uint32_t)(i) << 16)
#define EP_ERRORS(n)        ((uint32_t)(n) << 1)
#define EP_TYPE(t)          ((uint32_t)(t) << 3)
#define EP_MAX_PACKET(p)    ((uint32_t)(p) << 16)
#define EP_MAX_PACKET_MASK  0xFFFF0000UL
#define EP_AVERAGE_TRB(l)   ((uint32_t)(l) << 0)
#define EP_MAX_ESIT(l)      ((uint32_t)(l) << 16)
#define EP_TYPE_CONTROL     4
#define EP_TYPE_INTERRUPT_IN 7
#define EP_MAX_ERRORS       3

#define DCI_SLOT            0
#define DCI_CONTROL         1
#define DCI_IN(ep)          ((ep) * 2 + 1)

#define PAGE                0x1000
#define RING_TRBS           (PAGE / sizeof(Trb))
#define MAX_SLOTS           16
#define REPORT_SIZE         64
#define PIPE_QUEUE          32
#define COMMAND_TIMEOUT_MS  1000
#define CONTROL_TIMEOUT_MS  1000
#define PORT_TIMEOUT_MS     200
#define CONNECT_TIMEOUT_MS  500
#define RESET_TIMEOUT_MS    1000
#define RESET_RECOVERY_MS   20

typedef struct
{
    uint32_t dw[4];
} Trb;

typedef struct
{
    volatile Trb *trbs;
    uint32_t index;
    uint32_t cycle;
} Ring;

typedef struct
{
    bool used;
    Ring control;
    volatile uint32_t *context;
    uint8_t speed;
    uint32_t route;
} Device;

typedef struct
{
    Ring ring;
    uint8_t slot;
    uint8_t dci;
    uint16_t size;
    volatile uint8_t *buffer;
    uint8_t oldest;
    uint8_t completed;
    uint8_t codes[PIPE_QUEUE];
    uint16_t residuals[PIPE_QUEUE];
} Pipe;

static uintptr_t capRegs;
static uintptr_t opRegs;
static uintptr_t rtRegs;
static uintptr_t dbRegs;

static uint8_t *dmaBase;
static uint64_t dmaBusBase;
static size_t dmaSizeTotal;
static size_t dmaUsed = 0;

static uint32_t contextSize;
static volatile uint64_t *dcbaa;
static Ring commands;
static volatile Trb *events;
static uint32_t eventIndex = 0;
static uint32_t eventCycle = 1;
static volatile uint32_t *input;
static volatile uint8_t *controlBuffer;

static Device devices[MAX_SLOTS];
static Pipe pipes[USB_MAX_PIPES];
static uint8_t pipeCount = 0;

static bool commandDone;
static uint8_t commandCode;
static uint8_t commandSlot;

static uint8_t controlSlot = 0;
static uint64_t controlData;
static uint64_t controlStatus;
static bool controlDone;
static uint8_t controlCode;
static uint32_t controlResidual;

static UsbHost host;

static void Log(const char *fmt, int a, int b)
{
    char msg[64];
    snprintf(msg, sizeof(msg), fmt, a, b);
    svcDebug(msg);
}

static uint32_t Rd32(uintptr_t reg)
{
    return *(volatile uint32_t*)reg;
}

static void Wr32(uintptr_t reg, uint32_t value)
{
    *(volatile uint32_t*)reg = value;
}

static void Wr64(uintptr_t reg, uint64_t value)
{
    Wr32(reg, (uint32_t)value);
    Wr32(reg + 4, (uint32_t)(value >> 32));
}

static uint64_t Bus(volatile const void *ptr)
{
    return dmaBusBase + ((const volatile uint8_t*)ptr - dmaBase);
}

static void *Alloc(size_t size)
{
    size = (size + PAGE - 1) & ~(size_t)(PAGE - 1);
    if(dmaUsed + size > dmaSizeTotal)
        return nullptr;

    void *ptr = dmaBase + dmaUsed;
    dmaUsed += size;
    memset(ptr, 0, size);
    return ptr;
}

static bool WaitReg(uintptr_t reg, uint32_t mask, uint32_t value, uint32_t timeoutMs)
{
    uint64_t start = Counter();
    uint64_t ticks = CounterFreq() * timeoutMs / 1000;
    while((Rd32(reg) & mask) != value)
    {
        if(Counter() - start > ticks)
            return false;
        svcYield();
    }
    return true;
}

static bool Ring_Init(Ring *ring)
{
    ring->trbs = (volatile Trb*)Alloc(PAGE);
    if(!ring->trbs)
        return false;

    ring->index = 0;
    ring->cycle = TRB_CYCLE;

    uint64_t bus = Bus(ring->trbs);
    volatile Trb *link = &ring->trbs[RING_TRBS - 1];
    link->dw[0] = (uint32_t)bus;
    link->dw[1] = (uint32_t)(bus >> 32);
    link->dw[3] = TRB_TYPE(TYPE_LINK) | TRB_TOGGLE_CYCLE;
    return true;
}

//Hands the link back eagerly, so the enqueue pointer can never rest on it; the VL805 mishandles a dequeue that does
static uint64_t Ring_Push(Ring *ring, uint32_t dw0, uint32_t dw1, uint32_t dw2, uint32_t dw3)
{
    volatile Trb *trb = &ring->trbs[ring->index];
    trb->dw[0] = dw0;
    trb->dw[1] = dw1;
    trb->dw[2] = dw2;
    trb->dw[3] = dw3 | ring->cycle;
    uint64_t bus = Bus(trb);

    if(++ring->index == RING_TRBS - 1)
    {
        volatile Trb *link = &ring->trbs[ring->index];
        link->dw[3] = (link->dw[3] & ~(uint32_t)TRB_CYCLE) | ring->cycle;
        ring->cycle ^= TRB_CYCLE;
        ring->index = 0;
    }
    return bus;
}

static void RingDoorbell(uint8_t slot, uint32_t target)
{
    asm volatile("dsb sy");
    Wr32(dbRegs + slot * sizeof(uint32_t), target);
}

static void HandleTransferEvent(const Trb *event)
{
    uint64_t trb = event->dw[0] | ((uint64_t)event->dw[1] << 32);
    uint8_t code = event->dw[2] >> 24;
    uint32_t residual = event->dw[2] & 0xFFFFFF;
    uint8_t slot = event->dw[3] >> 24;
    uint8_t dci = (event->dw[3] >> 16) & 0x1F;

    if(dci == DCI_CONTROL)
    {
        if(slot != controlSlot)
            return;

        bool ok = code == CODE_SUCCESS || code == CODE_SHORT_PACKET;
        if(ok && trb == controlData)
            controlResidual = residual;
        if(!ok || trb == controlStatus)
        {
            controlCode = code;
            controlDone = true;
        }
        return;
    }

    for(uint8_t i = 0; i < pipeCount; i++)
    {
        if(pipes[i].slot == slot && pipes[i].dci == dci)
        {
            if(pipes[i].completed < PIPE_QUEUE)
            {
                uint8_t index = (pipes[i].oldest + pipes[i].completed++) % PIPE_QUEUE;
                pipes[i].codes[index] = code;
                pipes[i].residuals[index] = residual;
            }
            return;
        }
    }
}

static void DrainEvents()
{
    bool moved = false;
    while(1)
    {
        volatile Trb *next = &events[eventIndex];
        uint32_t control = next->dw[3];
        if((control & TRB_CYCLE) != eventCycle)
            break;

        asm volatile("dmb sy");
        Trb event = { { next->dw[0], next->dw[1], next->dw[2], control } };
        switch(TRB_GET_TYPE(control))
        {
            case TYPE_COMMAND_EVENT:
                commandCode = event.dw[2] >> 24;
                commandSlot = event.dw[3] >> 24;
                commandDone = true;
                break;
            case TYPE_TRANSFER_EVENT:
                HandleTransferEvent(&event);
                break;
            default:
                break;
        }

        moved = true;
        if(++eventIndex == RING_TRBS)
        {
            eventIndex = 0;
            eventCycle ^= TRB_CYCLE;
        }
    }

    if(moved)
        Wr64(rtRegs + RT_ERDP, Bus(&events[eventIndex]) | ERDP_BUSY);
}

static uint8_t Command(uint32_t dw0, uint32_t dw1, uint32_t dw2, uint32_t dw3)
{
    commandDone = false;
    Ring_Push(&commands, dw0, dw1, dw2, dw3);
    RingDoorbell(0, 0);

    uint64_t start = Counter();
    uint64_t ticks = CounterFreq() * COMMAND_TIMEOUT_MS / 1000;
    while(!commandDone)
    {
        DrainEvents();
        if(Counter() - start > ticks)
        {
            Log("xhci: command type %d timed out (status 0x%X)", TRB_GET_TYPE(dw3), Rd32(opRegs + OP_USBSTS));
            return 0;
        }
        svcYield();
    }

    if(commandCode != CODE_SUCCESS)
        Log("xhci: command type %d failed (code %d)", TRB_GET_TYPE(dw3), commandCode);
    return commandCode;
}

static uint8_t ContextCommand(uint32_t type, uint8_t slot)
{
    uint64_t bus = Bus(input);
    return Command((uint32_t)bus, (uint32_t)(bus >> 32), 0, TRB_TYPE(type) | TRB_SLOT(slot));
}

//Input contexts lead with a control context, which shifts every index by one
static volatile uint32_t *InputContext(uint32_t dci)
{
    return (volatile uint32_t*)((uintptr_t)input + (dci + 1) * contextSize);
}

static volatile uint32_t *OutputContext(uint8_t slot, uint32_t dci)
{
    return (volatile uint32_t*)((uintptr_t)devices[slot].context + dci * contextSize);
}

static void BeginInput(uint32_t addFlags)
{
    memset((void*)input, 0, PAGE);
    input[1] = addFlags;
}

static void CopyContext(volatile uint32_t *dst, volatile const uint32_t *src)
{
    for(uint32_t i = 0; i < contextSize / sizeof(uint32_t); i++)
        dst[i] = src[i];
}

static void RecoverEndpoint(uint8_t slot, uint32_t dci, Ring *ring)
{
    Command(0, 0, 0, TRB_TYPE(TYPE_RESET_EP) | TRB_SLOT(slot) | TRB_ENDPOINT(dci));

    uint64_t dequeue = Bus(&ring->trbs[ring->index]) | ring->cycle;
    Command((uint32_t)dequeue, (uint32_t)(dequeue >> 32), 0, TRB_TYPE(TYPE_SET_DEQUEUE) | TRB_SLOT(slot) | TRB_ENDPOINT(dci));
}

static bool Xhci_ResetPort(uint8_t port, UsbSpeed *speed)
{
    uintptr_t reg = opRegs + OP_PORTSC(port);
    uint32_t status = Rd32(reg);
    if(!(status & PORTSC_CONNECTED))
        return false;

    uint32_t portSpeed = PORTSC_SPEED(status);
    if(portSpeed > SPEED_HIGH)
    {
        Log("xhci: skipping superspeed port %d", port, 0);
        return false;
    }

    Wr32(reg, (status & PORTSC_PRESERVE) | PORTSC_RESET);
    if(!WaitReg(reg, PORTSC_RESET_CHANGE, PORTSC_RESET_CHANGE, PORT_TIMEOUT_MS))
    {
        Log("xhci: port %d didn't finish reset", port, 0);
        return false;
    }

    status = Rd32(reg);
    Wr32(reg, (status & PORTSC_PRESERVE) | (status & PORTSC_CHANGES));
    Usb_Delay(RESET_RECOVERY_MS);

    status = Rd32(reg);
    if(!(status & PORTSC_ENABLED))
    {
        Log("xhci: port %d didn't enable", port, 0);
        return false;
    }

    portSpeed = PORTSC_SPEED(status);
    Log("xhci: device on port %d (speed %d)", port, portSpeed);
    *speed = portSpeed == SPEED_HIGH ? USB_SPEED_HIGH : portSpeed == SPEED_LOW ? USB_SPEED_LOW : USB_SPEED_FULL;
    return true;
}

static uint32_t SlotSpeed(uint8_t speed)
{
    return speed == USB_SPEED_HIGH ? SPEED_HIGH : speed == USB_SPEED_LOW ? SPEED_LOW : SPEED_FULL;
}

static bool Xhci_Attach(UsbEndpoint *control, const UsbLocation *location)
{
    if(Command(0, 0, 0, TRB_TYPE(TYPE_ENABLE_SLOT)) != CODE_SUCCESS)
        return false;

    uint8_t slot = commandSlot;
    if(slot == 0 || slot >= MAX_SLOTS || devices[slot].used)
        return false;

    Device *device = &devices[slot];
    device->context = (volatile uint32_t*)Alloc(PAGE);
    if(!device->context || !Ring_Init(&device->control))
        return false;
    device->used = true;
    device->speed = location->speed;
    device->route = location->route;
    dcbaa[slot] = Bus(device->context);

    uint16_t maxPacket = location->speed == USB_SPEED_HIGH ? 64 : 8;
    BeginInput((1 << DCI_SLOT) | (1 << DCI_CONTROL));

    volatile uint32_t *slotContext = InputContext(DCI_SLOT);
    slotContext[0] = (location->route & SLOT_ROUTE_MASK) | SLOT_SPEED(SlotSpeed(location->speed)) | SLOT_ENTRIES(DCI_CONTROL);
    slotContext[1] = SLOT_ROOT_PORT(location->rootPort);
    if(location->ttSlot)
        slotContext[2] = SLOT_TT_HUB(location->ttSlot) | SLOT_TT_PORT(location->ttPort);

    uint64_t dequeue = Bus(device->control.trbs) | device->control.cycle;
    volatile uint32_t *endpoint = InputContext(DCI_CONTROL);
    endpoint[1] = EP_ERRORS(EP_MAX_ERRORS) | EP_TYPE(EP_TYPE_CONTROL) | EP_MAX_PACKET(maxPacket);
    endpoint[2] = (uint32_t)dequeue;
    endpoint[3] = (uint32_t)(dequeue >> 32);
    endpoint[4] = EP_AVERAGE_TRB(8);

    if(ContextCommand(TYPE_ADDRESS_DEVICE, slot) != CODE_SUCCESS)
        return false;

    control->address = slot;
    control->maxPacket = maxPacket;
    Usb_Delay(2);
    return true;
}

static bool Xhci_Assign(UsbEndpoint *control, uint16_t maxPacket)
{
    if(maxPacket == control->maxPacket)
        return true;

    BeginInput(1 << DCI_CONTROL);
    volatile uint32_t *endpoint = InputContext(DCI_CONTROL);
    CopyContext(endpoint, OutputContext(control->address, DCI_CONTROL));
    endpoint[1] = (endpoint[1] & ~EP_MAX_PACKET_MASK) | EP_MAX_PACKET(maxPacket);

    if(ContextCommand(TYPE_EVALUATE, control->address) != CODE_SUCCESS)
        return false;
    control->maxPacket = maxPacket;
    return true;
}

static bool Xhci_ConfigureHub(const UsbEndpoint *control, uint8_t ports, uint8_t thinkTime)
{
    BeginInput(1 << DCI_SLOT);
    volatile uint32_t *slotContext = InputContext(DCI_SLOT);
    CopyContext(slotContext, OutputContext(control->address, DCI_SLOT));
    slotContext[0] |= SLOT_HUB;
    slotContext[1] |= SLOT_PORTS(ports);
    if(control->speed == USB_SPEED_HIGH)
        slotContext[2] |= SLOT_TT_THINK(thinkTime);

    return ContextCommand(TYPE_CONFIGURE_EP, control->address) == CODE_SUCCESS;
}

//The VL805's internal hub can babble if a full speed control transfer starts late in microframe 7
static void AvoidHubBabble(const Device *device)
{
    if(device->speed != USB_SPEED_FULL || (device->route & 0xFFFF0))
        return;

    for(int i = 0; i < 2000 && (Rd32(rtRegs + RT_MFINDEX) & 7) == 0; i++);
}

static int Xhci_Control(const UsbEndpoint *ep, const UsbSetup *setup, void *data)
{
    size_t length = setup->length;
    if(length > USB_MAX_CONTROL || ep->address >= MAX_SLOTS || !devices[ep->address].used)
        return -1;

    bool in = setup->requestType & 0x80;
    Device *device = &devices[ep->address];
    if(length && !in)
        memcpy((void*)controlBuffer, data, length);

    uint32_t setupWords[2];
    memcpy(setupWords, setup, sizeof(setupWords));

    controlSlot = ep->address;
    controlDone = false;
    controlCode = 0;
    controlResidual = 0;
    controlData = 0;

    uint32_t transfer = !length ? 0 : in ? TRB_SETUP_IN : TRB_SETUP_OUT;
    Ring_Push(&device->control, setupWords[0], setupWords[1], sizeof(UsbSetup), TRB_TYPE(TYPE_SETUP) | TRB_IMMEDIATE | transfer);
    if(length)
    {
        uint64_t bus = Bus(controlBuffer);
        controlData = Ring_Push(&device->control, (uint32_t)bus, (uint32_t)(bus >> 32), length,
            TRB_TYPE(TYPE_DATA) | TRB_SHORT_INTERRUPT | TRB_IOC | (in ? TRB_DIR_IN : 0));
    }
    controlStatus = Ring_Push(&device->control, 0, 0, 0, TRB_TYPE(TYPE_STATUS) | TRB_IOC | ((in && length) ? 0 : TRB_DIR_IN));

    AvoidHubBabble(device);
    RingDoorbell(ep->address, DCI_CONTROL);

    uint64_t start = Counter();
    uint64_t ticks = CounterFreq() * CONTROL_TIMEOUT_MS / 1000;
    while(!controlDone)
    {
        DrainEvents();
        if(Counter() - start > ticks)
        {
            Log("xhci: control request %d timed out", setup->request, 0);
            return -1;
        }
        svcYield();
    }

    if(controlCode != CODE_SUCCESS && controlCode != CODE_SHORT_PACKET)
    {
        Log("xhci: control request %d failed (code %d)", setup->request, controlCode);
        RecoverEndpoint(ep->address, DCI_CONTROL, &device->control);
        return -1;
    }

    int transferred = length - controlResidual;
    if(length && in)
    {
        asm volatile("dsb sy");
        memcpy(data, (const void*)controlBuffer, transferred);
    }
    return transferred;
}

static void QueueTransfer(Pipe *pipe, uint8_t index)
{
    uint64_t bus = Bus(pipe->buffer + index * REPORT_SIZE);
    Ring_Push(&pipe->ring, (uint32_t)bus, (uint32_t)(bus >> 32), pipe->size, TRB_TYPE(TYPE_NORMAL) | TRB_SHORT_INTERRUPT | TRB_IOC);
}

//hid only polls once per frame; the controller keeps collecting reports in between
static void FillPipe(Pipe *pipe)
{
    pipe->oldest = 0;
    pipe->completed = 0;
    for(uint8_t i = 0; i < PIPE_QUEUE; i++)
        QueueTransfer(pipe, i);
    RingDoorbell(pipe->slot, pipe->dci);
}

static uint32_t IntervalExponent(uint8_t speed, uint8_t interval)
{
    if(speed == USB_SPEED_HIGH)
    {
        uint32_t exponent = interval ? interval - 1 : 0;
        return exponent > 15 ? 15 : exponent;
    }

    uint32_t exponent = 3;
    while(exponent < 10 && (1U << (exponent + 1)) <= interval * 8U)
        exponent++;
    return exponent;
}

static bool Xhci_OpenPipe(UsbPipe *usbPipe, const UsbEndpoint *ep, uint8_t interval)
{
    if(pipeCount >= USB_MAX_PIPES || ep->maxPacket == 0 || ep->maxPacket > REPORT_SIZE)
        return false;

    Pipe *pipe = &pipes[pipeCount];
    pipe->buffer = (volatile uint8_t*)Alloc(PIPE_QUEUE * REPORT_SIZE);
    if(!pipe->buffer || !Ring_Init(&pipe->ring))
        return false;
    pipe->slot = ep->address;
    pipe->dci = DCI_IN(ep->endpoint);
    pipe->size = ep->maxPacket;

    BeginInput((1 << DCI_SLOT) | (1UL << pipe->dci));
    volatile uint32_t *slotContext = InputContext(DCI_SLOT);
    CopyContext(slotContext, OutputContext(pipe->slot, DCI_SLOT));
    if(pipe->dci > SLOT_GET_ENTRIES(slotContext[0]))
        slotContext[0] = (slotContext[0] & ~SLOT_ENTRIES_MASK) | SLOT_ENTRIES(pipe->dci);

    uint64_t dequeue = Bus(pipe->ring.trbs) | pipe->ring.cycle;
    volatile uint32_t *endpoint = InputContext(pipe->dci);
    endpoint[0] = EP_INTERVAL(IntervalExponent(ep->speed, interval));
    endpoint[1] = EP_ERRORS(EP_MAX_ERRORS) | EP_TYPE(EP_TYPE_INTERRUPT_IN) | EP_MAX_PACKET(ep->maxPacket);
    endpoint[2] = (uint32_t)dequeue;
    endpoint[3] = (uint32_t)(dequeue >> 32);
    endpoint[4] = EP_AVERAGE_TRB(ep->maxPacket) | EP_MAX_ESIT(ep->maxPacket);

    if(ContextCommand(TYPE_CONFIGURE_EP, pipe->slot) != CODE_SUCCESS)
        return false;

    usbPipe->ep = *ep;
    usbPipe->channel = pipeCount++;
    usbPipe->toggle = 0;
    FillPipe(pipe);
    return true;
}

static int Xhci_PollPipe(UsbPipe *usbPipe, void *data, size_t size)
{
    DrainEvents();

    Pipe *pipe = &pipes[usbPipe->channel];
    if(!pipe->completed)
        return 0;

    uint8_t index = pipe->oldest;
    uint8_t code = pipe->codes[index];
    if(code != CODE_SUCCESS && code != CODE_SHORT_PACKET && code != CODE_MISSED_SERVICE)
    {
        //A halt strands every transfer behind the failed one, so the queue starts over
        Log("xhci: pipe %d failed (code %d)", usbPipe->channel, code);
        RecoverEndpoint(pipe->slot, pipe->dci, &pipe->ring);
        FillPipe(pipe);
        return 0;
    }

    int received = 0;
    if(code != CODE_MISSED_SERVICE)
    {
        asm volatile("dsb sy");
        received = pipe->size - pipe->residuals[index];
        if((size_t)received > size)
            received = size;
        memcpy(data, (const void*)(pipe->buffer + index * REPORT_SIZE), received);
    }

    pipe->oldest = (index + 1) % PIPE_QUEUE;
    pipe->completed--;
    QueueTransfer(pipe, index);
    RingDoorbell(pipe->slot, pipe->dci);
    return received;
}

static bool Xhci_Reset()
{
    Wr32(opRegs + OP_USBCMD, Rd32(opRegs + OP_USBCMD) & ~(uint32_t)USBCMD_RUN);
    if(!WaitReg(opRegs + OP_USBSTS, USBSTS_HALTED, USBSTS_HALTED, RESET_TIMEOUT_MS))
        return false;

    Wr32(opRegs + OP_USBCMD, USBCMD_RESET);
    return WaitReg(opRegs + OP_USBCMD, USBCMD_RESET, 0, RESET_TIMEOUT_MS) &&
           WaitReg(opRegs + OP_USBSTS, USBSTS_NOT_READY, 0, RESET_TIMEOUT_MS);
}

static bool Xhci_SetupScratchpad(uint32_t count)
{
    if(!count)
        return true;

    volatile uint64_t *array = (volatile uint64_t*)Alloc(count * sizeof(uint64_t));
    if(!array)
        return false;

    for(uint32_t i = 0; i < count; i++)
    {
        void *page = Alloc(PAGE);
        if(!page)
            return false;
        array[i] = Bus(page);
    }

    dcbaa[0] = Bus(array);
    return true;
}

static bool Xhci_SetupMemory(uint32_t scratchpads)
{
    dcbaa = (volatile uint64_t*)Alloc(PAGE);
    events = (volatile Trb*)Alloc(PAGE);
    volatile uint64_t *erst = (volatile uint64_t*)Alloc(PAGE);
    input = (volatile uint32_t*)Alloc(PAGE);
    controlBuffer = (volatile uint8_t*)Alloc(USB_MAX_CONTROL);
    if(!dcbaa || !events || !erst || !input || !controlBuffer || !Ring_Init(&commands))
        return false;
    if(!Xhci_SetupScratchpad(scratchpads))
        return false;

    erst[0] = Bus(events);
    erst[1] = RING_TRBS;

    Wr64(opRegs + OP_DCBAAP, Bus(dcbaa));
    Wr64(opRegs + OP_CRCR, Bus(commands.trbs) | commands.cycle);
    Wr32(rtRegs + RT_ERSTSZ, 1);
    Wr64(rtRegs + RT_ERDP, Bus(events));
    Wr64(rtRegs + RT_ERSTBA, Bus(erst));
    return true;
}

static void Xhci_PowerPorts(uint8_t ports)
{
    for(uint8_t port = 1; port <= ports; port++)
    {
        uintptr_t reg = opRegs + OP_PORTSC(port);
        uint32_t status = Rd32(reg);
        if(!(status & PORTSC_POWER))
            Wr32(reg, (status & PORTSC_PRESERVE) | PORTSC_POWER);
    }

    uint64_t start = Counter();
    uint64_t ticks = CounterFreq() * CONNECT_TIMEOUT_MS / 1000;
    while(Counter() - start < ticks)
    {
        for(uint8_t port = 1; port <= ports; port++)
            if(Rd32(opRegs + OP_PORTSC(port)) & PORTSC_CONNECTED)
                return;
        svcYield();
    }
}

const UsbHost *Xhci_Init(uintptr_t regs, uint8_t *dma, uint64_t dmaBus, size_t dmaSize)
{
    capRegs = regs;
    dmaBase = dma;
    dmaBusBase = dmaBus;
    dmaSizeTotal = dmaSize;

    uint32_t length = Rd32(capRegs + CAP_LENGTH);
    uint32_t params1 = Rd32(capRegs + CAP_HCSPARAMS1);
    uint32_t params2 = Rd32(capRegs + CAP_HCSPARAMS2);
    uint32_t caps = Rd32(capRegs + CAP_HCCPARAMS1);
    opRegs = capRegs + (length & 0xFF);
    rtRegs = capRegs + (Rd32(capRegs + CAP_RTSOFF) & ~0x1FU);
    dbRegs = capRegs + (Rd32(capRegs + CAP_DBOFF) & ~3U);
    contextSize = (caps & HCC1_CONTEXT_64) ? 64 : 32;

    uint32_t slots = HCS1_SLOTS(params1);
    uint8_t ports = HCS1_PORTS(params1);
    uint32_t scratchpads = HCS2_SCRATCH(params2);
    Log("xhci: version 0x%X, %d ports", length >> 16, ports);
    Log("xhci: %d slots, %d scratchpads", slots, scratchpads);

    if(!Xhci_Reset())
    {
        svcDebug("xhci: controller didn't reset");
        return nullptr;
    }

    Wr32(opRegs + OP_CONFIG, slots < MAX_SLOTS - 1 ? slots : MAX_SLOTS - 1);
    if(!Xhci_SetupMemory(scratchpads))
    {
        svcDebug("xhci: out of DMA memory");
        return nullptr;
    }

    Wr32(opRegs + OP_USBCMD, USBCMD_RUN);
    if(!WaitReg(opRegs + OP_USBSTS, USBSTS_HALTED, 0, RESET_TIMEOUT_MS))
    {
        svcDebug("xhci: controller didn't start");
        return nullptr;
    }
    Xhci_PowerPorts(ports);

    host.portCount = ports;
    host.hasTransactionTranslators = true;
    host.ResetPort = Xhci_ResetPort;
    host.Attach = Xhci_Attach;
    host.Assign = Xhci_Assign;
    host.ConfigureHub = Xhci_ConfigureHub;
    host.Control = Xhci_Control;
    host.OpenPipe = Xhci_OpenPipe;
    host.PollPipe = Xhci_PollPipe;
    return &host;
}
