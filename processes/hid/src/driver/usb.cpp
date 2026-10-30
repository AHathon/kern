#include "driver/usb.h"
#include "libraries/services/svc.h"
#include "libraries/libc/stdio.h"

#define REQ_IN                  0x80
#define REQ_CLASS               0x20
#define REQ_INTERFACE           0x01
#define REQ_OTHER               0x03

#define REQ_CLEAR_FEATURE       1
#define REQ_SET_FEATURE         3
#define REQ_SET_ADDRESS         5
#define REQ_GET_DESCRIPTOR      6
#define REQ_SET_CONFIGURATION   9
#define REQ_GET_STATUS          0

#define HID_SET_IDLE            0x0A
#define HID_SET_PROTOCOL        0x0B
#define HID_PROTOCOL_BOOT       0

#define DESC_DEVICE             1
#define DESC_CONFIG             2
#define DESC_INTERFACE          4
#define DESC_ENDPOINT           5
#define DESC_HUB                0x29

#define CLASS_HID               3
#define CLASS_HUB               9
#define HID_SUBCLASS_BOOT       1

#define EP_IN                   0x80
#define EP_TYPE_MASK            3
#define EP_TYPE_INTERRUPT       3

#define HUB_FEATURE_RESET       4
#define HUB_FEATURE_POWER       8
#define HUB_FEATURE_C_RESET     20
#define HUB_STATUS_CONNECTED    (1 << 0)
#define HUB_STATUS_ENABLED      (1 << 1)
#define HUB_STATUS_RESET        (1 << 4)
#define HUB_STATUS_LOW_SPEED    (1 << 9)
#define HUB_STATUS_HIGH_SPEED   (1 << 10)

#define MAX_HUB_DEPTH           5
#define MAX_ADDRESS             127
#define HUB_RESET_POLLS         50

typedef struct
{
    uint8_t length;
    uint8_t type;
    uint16_t usb;
    uint8_t deviceClass;
    uint8_t subClass;
    uint8_t protocol;
    uint8_t maxPacket0;
    uint16_t vendor;
    uint16_t product;
    uint16_t release;
    uint8_t strings[3];
    uint8_t configCount;
} __attribute__((packed)) UsbDeviceDesc;

typedef struct
{
    uint8_t length;
    uint8_t type;
    uint16_t totalLength;
    uint8_t interfaceCount;
    uint8_t value;
    uint8_t strings;
    uint8_t attributes;
    uint8_t maxPower;
} __attribute__((packed)) UsbConfigDesc;

typedef struct
{
    uint8_t length;
    uint8_t type;
    uint8_t number;
    uint8_t alternate;
    uint8_t endpointCount;
    uint8_t interfaceClass;
    uint8_t subClass;
    uint8_t protocol;
    uint8_t name;
} __attribute__((packed)) UsbInterfaceDesc;

typedef struct
{
    uint8_t length;
    uint8_t type;
    uint8_t address;
    uint8_t attributes;
    uint16_t maxPacket;
    uint8_t interval;
} __attribute__((packed)) UsbEndpointDesc;

typedef struct
{
    uint8_t length;
    uint8_t type;
    uint8_t portCount;
    uint16_t characteristics;
    uint8_t powerGood2ms;
} __attribute__((packed)) UsbHubDesc;

typedef struct
{
    uint16_t status;
    uint16_t change;
} UsbPortStatus;

static UsbHidDevice *found = nullptr;
static size_t foundCount = 0;
static size_t foundMax = 0;
static uint8_t nextAddress = 1;

static void EnumerateDevice(UsbSpeed speed, int depth);

static int Request(const UsbEndpoint *ep, uint8_t type, uint8_t request, uint16_t value, uint16_t index, void *data, uint16_t length)
{
    UsbSetup setup = { type, request, value, index, length };
    return Dwc2_Control(ep, &setup, data);
}

static int GetDescriptor(const UsbEndpoint *ep, uint8_t type, uint8_t classBits, void *data, uint16_t length)
{
    return Request(ep, REQ_IN | classBits, REQ_GET_DESCRIPTOR, (uint16_t)type << 8, 0, data, length);
}

static void Report(const char *fmt, int a, int b)
{
    char msg[64];
    snprintf(msg, sizeof(msg), fmt, a, b);
    svcDebug(msg);
}

static void AddHidDevice(const UsbEndpoint *control, const UsbInterfaceDesc *intf, const UsbEndpointDesc *desc)
{
    if(foundCount >= foundMax)
        return;

    Request(control, REQ_CLASS | REQ_INTERFACE, HID_SET_PROTOCOL, HID_PROTOCOL_BOOT, intf->number, nullptr, 0);
    //Optional for mice, so a stall isn't a failure
    Request(control, REQ_CLASS | REQ_INTERFACE, HID_SET_IDLE, 0, intf->number, nullptr, 0);

    UsbEndpoint ep = { control->address, (uint8_t)(desc->address & 0xF), control->speed, (uint16_t)(desc->maxPacket & 0x7FF) };
    UsbHidDevice *device = &found[foundCount];
    device->protocol = intf->protocol;
    if(!Dwc2_OpenPipe(&device->pipe, &ep))
    {
        svcDebug("out of host channels");
        return;
    }

    foundCount++;
    Report(intf->protocol == USB_PROTOCOL_KEYBOARD ? "keyboard at address %d ep %d" : "mouse at address %d ep %d", ep.address, ep.endpoint);
}

static void ScanInterfaces(const UsbEndpoint *control, const uint8_t *config, size_t size)
{
    const UsbInterfaceDesc *intf = nullptr;
    for(size_t off = 0; off + 2 <= size && config[off] >= 2; off += config[off])
    {
        if(off + config[off] > size)
            return;

        if(config[off + 1] == DESC_INTERFACE && config[off] >= sizeof(UsbInterfaceDesc))
        {
            intf = (const UsbInterfaceDesc*)(config + off);
            if(intf->alternate != 0 || intf->interfaceClass != CLASS_HID || intf->subClass != HID_SUBCLASS_BOOT ||
                (intf->protocol != USB_PROTOCOL_KEYBOARD && intf->protocol != USB_PROTOCOL_MOUSE))
                intf = nullptr;
        }
        else if(intf && config[off + 1] == DESC_ENDPOINT && config[off] >= sizeof(UsbEndpointDesc))
        {
            const UsbEndpointDesc *desc = (const UsbEndpointDesc*)(config + off);
            if((desc->address & EP_IN) && (desc->attributes & EP_TYPE_MASK) == EP_TYPE_INTERRUPT)
            {
                AddHidDevice(control, intf, desc);
                intf = nullptr;
            }
        }
    }
}

static void EnumerateHub(const UsbEndpoint *control, int depth)
{
    UsbHubDesc hub;
    if(GetDescriptor(control, DESC_HUB, REQ_CLASS, &hub, sizeof(hub)) < (int)sizeof(hub))
        return;

    for(uint8_t port = 1; port <= hub.portCount; port++)
        Request(control, REQ_CLASS | REQ_OTHER, REQ_SET_FEATURE, HUB_FEATURE_POWER, port, nullptr, 0);
    Dwc2_Delay(hub.powerGood2ms * 2 + 20);

    for(uint8_t port = 1; port <= hub.portCount; port++)
    {
        UsbPortStatus status;
        if(Request(control, REQ_IN | REQ_CLASS | REQ_OTHER, REQ_GET_STATUS, 0, port, &status, sizeof(status)) < (int)sizeof(status))
            continue;
        if(!(status.status & HUB_STATUS_CONNECTED))
            continue;

        Request(control, REQ_CLASS | REQ_OTHER, REQ_SET_FEATURE, HUB_FEATURE_RESET, port, nullptr, 0);
        for(int i = 0; i < HUB_RESET_POLLS; i++)
        {
            Dwc2_Delay(10);
            if(Request(control, REQ_IN | REQ_CLASS | REQ_OTHER, REQ_GET_STATUS, 0, port, &status, sizeof(status)) < (int)sizeof(status))
                status.status = 0;
            if(!(status.status & HUB_STATUS_RESET))
                break;
        }
        Request(control, REQ_CLASS | REQ_OTHER, REQ_CLEAR_FEATURE, HUB_FEATURE_C_RESET, port, nullptr, 0);
        if(!(status.status & HUB_STATUS_ENABLED))
            continue;
        Dwc2_Delay(10);

        UsbSpeed speed = (status.status & HUB_STATUS_LOW_SPEED) ? USB_SPEED_LOW :
                         (status.status & HUB_STATUS_HIGH_SPEED) ? USB_SPEED_HIGH : USB_SPEED_FULL;

        //A high speed hub talks to slower devices through split transactions, which aren't implemented
        if(control->speed == USB_SPEED_HIGH && speed != USB_SPEED_HIGH)
        {
            Report("skipping hub port %d: needs split transactions", port, 0);
            continue;
        }
        EnumerateDevice(speed, depth + 1);
    }
}

static void EnumerateDevice(UsbSpeed speed, int depth)
{
    if(nextAddress > MAX_ADDRESS)
        return;

    //Packet size is unknown until the first 8 bytes of the descriptor are in
    UsbEndpoint control = { 0, 0, (uint8_t)speed, 8 };
    UsbDeviceDesc device;
    if(GetDescriptor(&control, DESC_DEVICE, 0, &device, 8) < 8 || device.maxPacket0 == 0)
    {
        svcDebug("device didn't answer");
        return;
    }
    control.maxPacket = device.maxPacket0;

    uint8_t address = nextAddress;
    if(Request(&control, 0, REQ_SET_ADDRESS, address, 0, nullptr, 0) < 0)
    {
        svcDebug("couldn't set device address");
        return;
    }
    nextAddress++;
    control.address = address;
    Dwc2_Delay(10);

    static uint8_t config[DWC2_MAX_CONTROL];
    if(GetDescriptor(&control, DESC_DEVICE, 0, &device, sizeof(device)) < (int)sizeof(device) ||
        GetDescriptor(&control, DESC_CONFIG, 0, config, sizeof(UsbConfigDesc)) < (int)sizeof(UsbConfigDesc))
    {
        svcDebug("couldn't read descriptors");
        return;
    }

    const UsbConfigDesc *desc = (const UsbConfigDesc*)config;
    uint8_t value = desc->value;
    uint16_t total = desc->totalLength > sizeof(config) ? sizeof(config) : desc->totalLength;
    int size = GetDescriptor(&control, DESC_CONFIG, 0, config, total);
    if(size < (int)sizeof(UsbConfigDesc) || Request(&control, 0, REQ_SET_CONFIGURATION, value, 0, nullptr, 0) < 0)
    {
        svcDebug("couldn't configure device");
        return;
    }

    if(device.deviceClass == CLASS_HUB)
    {
        if(depth < MAX_HUB_DEPTH)
            EnumerateHub(&control, depth);
        return;
    }
    ScanInterfaces(&control, config, size);
}

size_t Usb_Enumerate(UsbHidDevice *devices, size_t maxDevices)
{
    found = devices;
    foundCount = 0;
    foundMax = maxDevices;

    UsbSpeed speed;
    if(Dwc2_ResetPort(&speed))
        EnumerateDevice(speed, 0);
    return foundCount;
}
