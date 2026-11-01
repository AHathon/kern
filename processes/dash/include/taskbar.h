#pragma once

#include "image.h"
#include "window.h"
#include "start_menu.h"

class Taskbar
{
public:
    Taskbar() = default;
    ~Taskbar() = default;

    void Initialize();
    void Finalize();
    void Draw(Window **windows, int windowCount, StartMenu *startMenu);

    bool OrbHovered() const { return orbHovered; }

private:
    void DrawOrb(StartMenu *startMenu);
    void DrawWindowButtons(Window **windows, int windowCount, float right);
    float DrawTray(Window **windows, int windowCount);

    Image orb;
    bool orbHovered = false;
};
