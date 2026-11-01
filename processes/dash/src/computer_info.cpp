#include "theme.h"
#include "computer_info.h"
#include "services.h"
#include "libraries/hardware/constants.h"

void ComputerInfo::DrawContent()
{
    HidState state = {};
    hidGetState(&hid, &state);

    ImGui::Text("kern");
    ImGui::Separator();
    ImGui::Text("Display: %d x %d", MAIN_DISP_WIDTH, MAIN_DISP_HEIGHT);
    ImGui::Text("Keyboard: %s", (state.devices & HID_DEVICE_KEYBOARD) ? "connected" : "none");
    ImGui::Text("Mouse: %s", (state.devices & HID_DEVICE_MOUSE) ? "connected" : "none");
}
