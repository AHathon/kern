#include "libraries/ipc/hid.h"
#include "hid.h"

long hidInitialize(Service *hid)
{
    return serviceOpen(hid, HID_PORT_NAME);
}

void hidExit(Service *hid)
{
    serviceClose(hid);
}

long hidGetState(Service *hid, HidState *state)
{
    return serviceDispatch(hid, HID_CMD_GET_STATE, 0, 0, state, sizeof(*state), 0, 0);
}
