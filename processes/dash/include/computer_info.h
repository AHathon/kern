#pragma once

#include "window.h"

class ComputerInfo : public Window
{
public:
    ComputerInfo() : Window("Computer", 330, 150, 300, 170, false) {}

protected:
    void DrawContent() override;
};
