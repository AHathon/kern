#pragma once

#include "imgui/imgui.h"
#include "libraries/ipc/gfx.h"
#include "libraries/ipc/hid.h"

void ImGuiBackend_Init();
void ImGuiBackend_Shutdown();
void ImGuiBackend_SetClearColor(uint32_t color);
void ImGuiBackend_UpdateInput(const HidState *state);
void ImGuiBackend_Render(ImDrawData *data);
