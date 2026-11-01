#pragma once

#include "driver/host.h"

#define USB_PROTOCOL_KEYBOARD   1
#define USB_PROTOCOL_MOUSE      2

typedef struct
{
    uint8_t protocol;
    UsbPipe pipe;
} UsbHidDevice;

size_t Usb_Enumerate(const UsbHost *host, UsbHidDevice *devices, size_t maxDevices);
