#pragma once

#include "libraries/types.h"

#define USB_MAX_PIPES       7
#define USB_MAX_CONTROL     1024

//Values of DWC2's HPRT.PRTSPD
typedef enum
{
    USB_SPEED_HIGH = 0,
    USB_SPEED_FULL = 1,
    USB_SPEED_LOW = 2
} UsbSpeed;

typedef struct
{
    uint8_t requestType;
    uint8_t request;
    uint16_t value;
    uint16_t index;
    uint16_t length;
} __attribute__((packed)) UsbSetup;

typedef struct
{
    uint8_t address;
    uint8_t endpoint;
    uint8_t speed;
    uint16_t maxPacket;
} UsbEndpoint;

typedef struct
{
    uint8_t speed;
    uint8_t rootPort;
    uint32_t route;
    uint8_t ttSlot;
    uint8_t ttPort;
} UsbLocation;

typedef struct
{
    UsbEndpoint ep;
    uint8_t channel;
    uint8_t toggle;
} UsbPipe;

typedef struct
{
    uint8_t portCount;
    bool hasTransactionTranslators;

    bool (*ResetPort)(uint8_t port, UsbSpeed *speed);
    bool (*Attach)(UsbEndpoint *control, const UsbLocation *location);
    bool (*Assign)(UsbEndpoint *control, uint16_t maxPacket);
    bool (*ConfigureHub)(const UsbEndpoint *control, uint8_t ports, uint8_t thinkTime);

    int (*Control)(const UsbEndpoint *ep, const UsbSetup *setup, void *data);

    //Interrupt IN pipes run one transaction at a time; each poll collects it and queues the next
    bool (*OpenPipe)(UsbPipe *pipe, const UsbEndpoint *ep, uint8_t interval);
    int (*PollPipe)(UsbPipe *pipe, void *data, size_t size);
} UsbHost;

void Usb_Delay(uint32_t ms);
