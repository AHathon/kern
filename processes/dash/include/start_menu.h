#pragma once

#include "window.h"

#define START_MENU_MAX_ENTRIES 16

class StartMenu
{
public:
    StartMenu() = default;
    ~StartMenu() = default;

    void AddProgram(Window *window);
    void AddPlace(Window *window);

    void Toggle();
    void Close() { open = false; }

    void Draw(bool orbHovered);

private:
    void DrawPrograms(float x0, float y0, float x1, float y1);
    void DrawPlaces(float x0, float y0, float x1, float y1);

    Window *programs[START_MENU_MAX_ENTRIES];
    Window *places[START_MENU_MAX_ENTRIES];
    int programCount = 0;
    int placeCount = 0;
    char search[32] = "";
    bool open = false;
    bool focusSearch = false;
};
