#include "theme.h"
#include "window.h"

#define MIN_VISIBLE     80.0f

Window *Window::active = nullptr;

Window::Window(const char *title, float x, float y, float width, float height, bool open, float padding) :
    title(title), x(x), y(y), width(width), height(height), padding(padding), open(open)
{
}

void Window::Open()
{
    open = true;
    minimized = false;
    wantFocus = true;
}

void Window::Close()
{
    open = false;
    if(active == this)
        active = nullptr;
}

void Window::Minimize()
{
    minimized = true;
    if(active == this)
        active = nullptr;
}

void Window::ToggleMaximize()
{
    wantPlace = true;
    maximized = !maximized;
    if(!maximized)
    {
        x = restoreX;
        y = restoreY;
        width = restoreWidth;
        height = restoreHeight;
        return;
    }

    restoreX = x;
    restoreY = y;
    restoreWidth = width;
    restoreHeight = height;

    ImVec2 screen = ImGui::GetIO().DisplaySize;
    x = 0;
    y = 0;
    width = screen.x;
    height = screen.y - THEME_TASKBAR_HEIGHT;
}

void Window::ToggleFromTaskbar()
{
    if(IsActive() && !minimized)
        Minimize();
    else
        Open();
}

void Window::MoveTo(float newX, float newY)
{
    ImVec2 screen = ImGui::GetIO().DisplaySize;
    x = ImClamp(newX, MIN_VISIBLE - width, screen.x - MIN_VISIBLE);
    y = ImClamp(newY, 0.0f, screen.y - THEME_TASKBAR_HEIGHT - THEME_WINDOW_CAPTION);
    wantPlace = true;
}

void Window::DrawCaptionButtons(float right, float top)
{
    switch(Theme_CaptionButtons(right, top, IsActive()))
    {
        case THEME_CAPTION_MINIMIZE: Minimize(); break;
        case THEME_CAPTION_MAXIMIZE: ToggleMaximize(); break;
        case THEME_CAPTION_CLOSE: Close(); break;
        default: break;
    }
}

void Window::HandleCaption(float captionWidth)
{
    ImGui::InvisibleButton("##caption", ImVec2(captionWidth, THEME_WINDOW_CAPTION - 1));

    ImVec2 mouse = ImGui::GetIO().MousePos;
    if(ImGui::IsItemActivated())
    {
        grabX = mouse.x - x;
        grabY = mouse.y - y;
    }
    if(ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        ToggleMaximize();
    else if(ImGui::IsItemActive() && !maximized)
        MoveTo(mouse.x - grabX, mouse.y - grabY);
}

void Window::Draw()
{
    if(!Begin())
        return;

    DrawContent();
    End();
}

bool Window::Begin()
{
    if(!open || minimized)
        return false;

    if(wantPlace)
    {
        ImGui::SetNextWindowPos(ImVec2(x, y));
        ImGui::SetNextWindowSize(ImVec2(width, height));
        wantPlace = false;
    }
    if(wantFocus)
    {
        ImGui::SetNextWindowFocus();
        wantFocus = false;
    }

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground;
    if(maximized)
        flags |= ImGuiWindowFlags_NoResize;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin(title, nullptr, flags);
    ImGui::PopStyleVar();

    if(ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
        active = this;

    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();
    x = pos.x;
    y = pos.y;
    width = size.x;
    height = size.y;

    Theme_WindowFrame(pos, pos + size, title, IsActive(), maximized);
    DrawCaptionButtons(pos.x + size.x - THEME_WINDOW_ROUNDING, pos.y + 1);

    ImGui::SetCursorScreenPos(pos);
    HandleCaption(size.x - THEME_WINDOW_ROUNDING - THEME_CAPTION_BUTTONS_WIDTH);

    ImGui::SetCursorScreenPos(pos + ImVec2(THEME_WINDOW_FRAME, THEME_WINDOW_CAPTION));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, padding));
    ImGui::BeginChild("##client", size - ImVec2(THEME_WINDOW_FRAME * 2, THEME_WINDOW_CAPTION + THEME_WINDOW_FRAME), ImGuiChildFlags_AlwaysUseWindowPadding);
    ImGui::PopStyleVar();
    return true;
}

void Window::End()
{
    ImGui::EndChild();
    ImGui::End();
}
