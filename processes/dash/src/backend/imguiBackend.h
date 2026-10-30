#pragma once

#include "imgui/imgui.h"
#include "libraries/ipc/gfx.h"
#include "libraries/ipc/hid.h"

void ImGuiBackend_Init(Service *gfx);
void ImGuiBackend_UpdateInput(const HidState *state);
void ImGuiBackend_Render(ImDrawData *data);
