#include "theme.h"
#include "desktop.h"
#include "computer_info.h"
#include "file_browser.h"
#include "services.h"
#include "test_window.h"
#include "backend/imguiBackend.h"
#include "libraries/hardware/constants.h"
#include "libraries/services/svc.h"
#include <time.h>

extern const char openSansTtf[];
extern const char openSansTtfEnd[];

static void LoadFont()
{
    ImFontConfig config;
    config.FontDataOwnedByAtlas = false;
    ImGuiIO &io = ImGui::GetIO();
    io.FontDefault = io.Fonts->AddFontFromMemoryTTF((void*)openSansTtf, (int)(openSansTtfEnd - openSansTtf), THEME_FONT_SIZE, &config);
}

void Desktop::Initialize()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.DisplaySize = ImVec2(MAIN_DISP_WIDTH, MAIN_DISP_HEIGHT);
    ImGuiBackend_Init();

    Theme_Apply();
    LoadFont();
    Theme_LoadIcons();
    taskbar.Initialize();

    AddProgram(new TestWindow());
    AddProgram(new FileBrowser());
    
    AddPlace(new ComputerInfo());

    lastTick = Counter();
}

void Desktop::Finalize()
{
    for(int i = 0; i < windowCount; i++)
        delete windows[i];
    windowCount = 0;

    wallpaper.Finalize();
    taskbar.Finalize();
    Theme_UnloadIcons();
    ImGuiBackend_Shutdown();
    ImGui::DestroyContext();

    m_isRunning = false;
}

bool Desktop::AddWindow(Window *window)
{
    if(windowCount == DESKTOP_MAX_WINDOWS)
        return false;
    windows[windowCount++] = window;
    return true;
}

void Desktop::AddProgram(Window *window)
{
    if(AddWindow(window))
        startMenu.AddProgram(window);
}

void Desktop::AddPlace(Window *window)
{
    if(AddWindow(window))
        startMenu.AddPlace(window);
}

void Desktop::Update()
{
    HidState input = {};
    if(hidGetState(&hid, &input) == HID_OK)
        ImGuiBackend_UpdateInput(&input);

    uint64_t now = Counter();
    float deltaTime = (float)(now - lastTick) / (float)CounterFreq();
    if(deltaTime <= 0.0f)
        deltaTime = 1.0f / 1000.0f;
    lastTick = now;

    ImGui::GetIO().DeltaTime = deltaTime;
    ImGui::NewFrame();
    wallpaper.Draw();

    for(int i = 0; i < windowCount; i++)
        windows[i]->Draw();

    taskbar.Draw(windows, windowCount, &startMenu);
    startMenu.Draw(taskbar.OrbHovered());

    if(!ImGui::GetCurrentContext()->NavWindow)
        Window::ClearActive();

    ImGui::Render();
    ImGuiBackend_Render(ImGui::GetDrawData());
}
