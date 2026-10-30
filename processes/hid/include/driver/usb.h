#pragma once

#include "dwc2.h"

#define USB_PROTOCOL_KEYBOARD   1
#define USB_PROTOCOL_MOUSE      2

typedef struct
{
    uint8_t protocol;
    Dwc2Pipe pipe;
} UsbHidDevice;

size_t Usb_Enumerate(UsbHidDevice *devices, size_t maxDevices);
