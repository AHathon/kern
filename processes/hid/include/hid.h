#pragma once

#include "libraries/types.h"
#include "libraries/ipc/hid.h"

#define HID_PORT_NAME   "hid"

typedef enum
{
    HID_CMD_GET_STATE = 1
} HidCommand;
