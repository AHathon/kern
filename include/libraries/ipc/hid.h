#pragma once

#include "libraries/types.h"
#include "libraries/ipc/service.h"

#define HID_OK              0
#define HID_ERR_INVALID     -300

#define HID_DEVICE_KEYBOARD (1 << 0)
#define HID_DEVICE_MOUSE    (1 << 1)

#define HID_MOD_CTRL        0x11
#define HID_MOD_SHIFT       0x22
#define HID_MOD_ALT         0x44
#define HID_MOD_SUPER       0x88

#define HID_BUTTON_LEFT     (1 << 0)
#define HID_BUTTON_RIGHT    (1 << 1)
#define HID_BUTTON_MIDDLE   (1 << 2)

typedef struct
{
    uint32_t devices;
    uint32_t keys[8];
    uint8_t modifiers;
    uint8_t buttons;
    int32_t mouseX, mouseY;
    int32_t wheel;  //running total
} HidState;

static inline int hidKeyDown(const HidState *state, uint8_t usage)
{
    return (state->keys[usage / 32] >> (usage % 32)) & 1;
}

#ifdef __cplusplus
extern "C" {
#endif

long hidInitialize(Service *hid);
void hidExit(Service *hid);
long hidGetState(Service *hid, HidState *state);

#ifdef __cplusplus
}
#endif
