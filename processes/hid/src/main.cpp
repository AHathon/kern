#include "libraries/ipc/server.h"
#include "libraries/services/svc.h"
#include "libraries/hardware/constants.h"
#include "libc/string.h"
#include "hid.h"
#include "usb.h"

#define MAX_DEVICES         DWC2_MAX_PIPES
#define KEY_ROLLOVER_ERROR  0x01

static UsbHidDevice devices[MAX_DEVICES];
static size_t deviceCount = 0;
static HidState state;

static_assert(sizeof(HidState) <= sizeof(IpcMessage::data), "HidState doesn't fit in an IPC message");

static void HandleKeyboard(const uint8_t *report, int size)
{
    //Every slot reads ERR_ROLLOVER when too many keys are held; keep what we had
    if(size < 8 || report[2] == KEY_ROLLOVER_ERROR)
        return;

    state.modifiers = report[0];
    memset(state.keys, 0, sizeof(state.keys));
    for(int i = 2; i < 8; i++)
        if(report[i])
            state.keys[report[i] / 32] |= 1U << (report[i] % 32);
}

static int32_t Clamp(int32_t v, int32_t min, int32_t max)
{
    return v < min ? min : v > max ? max : v;
}

static void HandleMouse(const uint8_t *report, int size)
{
    if(size < 3)
        return;

    state.buttons = report[0];
    state.mouseX = Clamp(state.mouseX + (int8_t)report[1], 0, MAIN_DISP_WIDTH - 1);
    state.mouseY = Clamp(state.mouseY + (int8_t)report[2], 0, MAIN_DISP_HEIGHT - 1);
    if(size >= 4)
        state.wheel += (int8_t)report[3];
}

static void Hid_Poll()
{
    uint8_t report[8];
    for(size_t i = 0; i < deviceCount; i++)
    {
        int size = Dwc2_PollPipe(&devices[i].pipe, report, sizeof(report));
        if(size <= 0)
            continue;

        if(devices[i].protocol == USB_PROTOCOL_KEYBOARD)
            HandleKeyboard(report, size);
        else
            HandleMouse(report, size);
    }
}

static long Hid_Handle(ServerRequest *req, void *ctx)
{
    UNUSED(ctx);
    switch(req->cmd)
    {
        case HID_CMD_GET_STATE:
            Hid_Poll();
            memcpy(req->out, &state, sizeof(state));
            req->outSize = sizeof(state);
            return HID_OK;
        default:
            return HID_ERR_INVALID;
    }
}

int main()
{
    state.mouseX = MAIN_DISP_WIDTH / 2;
    state.mouseY = MAIN_DISP_HEIGHT / 2;

    //Clients wait on the port, so it has to come up even with nothing to drive
    if(Dwc2_Init())
        deviceCount = Usb_Enumerate(devices, MAX_DEVICES);
    else
        svcDebug("no usb controller");

    for(size_t i = 0; i < deviceCount; i++)
        state.devices |= devices[i].protocol == USB_PROTOCOL_KEYBOARD ? HID_DEVICE_KEYBOARD : HID_DEVICE_MOUSE;

    long port = serverRegister(HID_PORT_NAME);
    if(port < 0)
    {
        svcDebug("couldn't register port");
        return 0;
    }

    serverRun(port, Hid_Handle, nullptr);
    svcDebug("server stopped");
    return 0;
}
