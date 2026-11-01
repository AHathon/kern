#pragma once

#include "window.h"

class TestWindow : public Window
{
public:
    TestWindow() : Window("Test", 40, 30, 420, 250) {}

protected:
    void DrawContent() override;

private:
    float progress = 0.0f;
    char text[64] = "";
    int clicks = 0;
};
