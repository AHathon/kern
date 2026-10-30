#include "backend/imgui/imgui.h"
#include "backend/imguiBackend.h"
#include "libraries/services/svc.h"
#include "libraries/hardware/constants.h"
#include <time.h>

static uint64_t Counter()
{
    uint64_t v;
    asm volatile("isb; mrs %0, cntpct_el0" : "=r"(v));
    return v;
}

int main()
{
    Service gfx;
    if(gfxInitialize(&gfx) != GFX_OK)
    {
        svcDebug("couldn't connect to gfx");
        return 0;
    }

    Service hid;
    if(hidInitialize(&hid) != HID_OK)
        svcDebug("couldn't connect to hid");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.DisplaySize = ImVec2(MAIN_DISP_WIDTH, MAIN_DISP_HEIGHT);
    ImGuiBackend_Init(&gfx);

    uint64_t freq = CounterFreq();
    uint64_t last = Counter();
    float progress = 0.0f;
    char text[64] = "";
    int clicks = 0;
    HidState input = {};

    while(1)
    {
        if(hidGetState(&hid, &input) == HID_OK)
            ImGuiBackend_UpdateInput(&input);

        uint64_t now = Counter();
        io.DeltaTime = (float)(now - last) / (float)freq;
        if(io.DeltaTime <= 0.0f)
            io.DeltaTime = 1.0f / 1000.0f;
        last = now;

        progress += io.DeltaTime * 0.25f;
        if(progress > 1.0f)
            progress -= 1.0f;

        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(40, 40), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(420, 220), ImGuiCond_Once);
        ImGui::Begin("dash");
        ImGui::Text("Dear ImGui %s", IMGUI_VERSION);
        ImGui::Text("%.1f FPS", io.Framerate);
        ImGui::Separator();
        ImGui::ProgressBar(progress);
        ImGui::SliderFloat("progress", &progress, 0.0f, 1.0f);
        if(ImGui::Button("Button"))
            clicks++;
        ImGui::SameLine();
        ImGui::Text("clicked %d times", clicks);
        ImGui::InputText("text", text, sizeof(text));
        ImGui::Text("keyboard: %s  mouse: %s",
            (input.devices & HID_DEVICE_KEYBOARD) ? "yes" : "no",
            (input.devices & HID_DEVICE_MOUSE) ? "yes" : "no");
        ImGui::End();
        ImGui::Render();

        ImGuiBackend_Render(ImGui::GetDrawData());
    }
}
