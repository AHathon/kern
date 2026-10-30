#pragma once

#include "libraries/types.h"

#define DWC2_MAX_PIPES      7
#define DWC2_MAX_CONTROL    1024

//Values of HPRT.PRTSPD
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
    UsbEndpoint ep;
    uint8_t channel;
    uint8_t toggle;
} Dwc2Pipe;

void Dwc2_Delay(uint32_t ms);
bool Dwc2_Init();
bool Dwc2_ResetPort(UsbSpeed *speed);

int Dwc2_Control(const UsbEndpoint *ep, const UsbSetup *setup, void *data);

//Interrupt IN pipes run one transaction at a time; each poll collects it and queues the next
bool Dwc2_OpenPipe(Dwc2Pipe *pipe, const UsbEndpoint *ep);
int Dwc2_PollPipe(Dwc2Pipe *pipe, void *data, size_t size);
