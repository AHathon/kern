#pragma once

#include "imgui/imgui.h"
#include "libraries/ipc/gfx.h"

void ImGuiBackend_Init(Service *gfx);
void ImGuiBackend_Render(ImDrawData *data);
