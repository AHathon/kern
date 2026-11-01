#pragma once

#include "libraries/types.h"
#include "start_menu.h"
#include "taskbar.h"
#include "wallpaper.h"
#include "window.h"

#define DESKTOP_MAX_WINDOWS 16

class Desktop
{
public:
    Desktop() = default;
    ~Desktop() = default;

    void Initialize();
    void Finalize();
    bool IsRunning(){ return m_isRunning; }
    void Update();

    Wallpaper &GetWallpaper() { return wallpaper; }

private:
    bool AddWindow(Window *window);
    void AddProgram(Window *window);
    void AddPlace(Window *window);

    uint64_t lastTick = 0;

    Wallpaper wallpaper;
    Taskbar taskbar;
    StartMenu startMenu;
    Window *windows[DESKTOP_MAX_WINDOWS];
    int windowCount = 0;

    bool m_isRunning = true;
};
