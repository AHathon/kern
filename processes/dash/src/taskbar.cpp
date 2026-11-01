#include "theme.h"
#include "taskbar.h"
#include "libraries/libc/stdio.h"
#include "libraries/services/svc.h"

#define ORB_FRAMES      3.0f
#define ORB_MARGIN      2.0f
#define ORB_SLOT        46.0f
#define BUTTON_MAX      150.0f
#define BUTTON_GAP      3.0f
#define SHOW_DESKTOP    12.0f
#define CLOCK_WIDTH     66.0f

extern const char orbBmp[];
extern const char orbBmpEnd[];

void Taskbar::Initialize()
{
    if(!orb.LoadBmp(orbBmp, orbBmpEnd - orbBmp))
        svcDebug("couldn't load orb");
}

void Taskbar::Finalize()
{
    orb.Unload();
}

void Taskbar::DrawOrb(StartMenu *startMenu)
{
    ImVec2 screen = ImGui::GetIO().DisplaySize;
    ImVec2 pos(0, screen.y - THEME_TASKBAR_HEIGHT);

    ImGui::SetCursorScreenPos(pos);
    if(ImGui::InvisibleButton("##orb", ImVec2(ORB_SLOT, THEME_TASKBAR_HEIGHT)))
        startMenu->Toggle();
    orbHovered = ImGui::IsItemHovered();
    bool held = ImGui::IsItemActive();

    if(!orb.Texture())
        return;

    float size = (float)orb.Width();
    float frame = held ? 2.0f : (orbHovered ? 1.0f : 0.0f);
    ImVec2 min((ORB_SLOT - size) * 0.5f, screen.y - size + ORB_MARGIN);
    ImDrawList *list = ImGui::GetWindowDrawList();
    list->PushClipRectFullScreen();
    list->AddImage(ImTextureRef((ImTextureID)orb.Texture()), min, min + ImVec2(size, size),
        ImVec2(0, frame / ORB_FRAMES), ImVec2(1, (frame + 1.0f) / ORB_FRAMES));
    list->PopClipRect();
}

void Taskbar::DrawWindowButtons(Window **windows, int windowCount, float right)
{
    int shown = 0;
    for(int i = 0; i < windowCount; i++)
        if(windows[i]->IsOpen())
            shown++;
    if(shown == 0)
        return;

    float left = ORB_SLOT + 2;
    float width = ImMin(BUTTON_MAX, (right - left) / shown - BUTTON_GAP);
    if(width < 28.0f)
        return;

    float top = ImGui::GetCurrentWindow()->Pos.y + 2;
    float height = THEME_TASKBAR_HEIGHT - 3;

    for(int i = 0; i < windowCount; i++)
    {
        if(!windows[i]->IsOpen())
            continue;

        ImVec2 min(left, top);
        ImVec2 max(left + width, top + height);

        ImGui::SetCursorScreenPos(min);
        ImGui::PushID(i);
        if(ImGui::InvisibleButton("##task", ImVec2(width, height)))
            windows[i]->ToggleFromTaskbar();
        ImGui::PopID();

        Theme_TaskButton(min, max, windows[i]->Title(), ImGui::IsItemHovered(), windows[i]->IsActive());

        left += width + BUTTON_GAP;
    }
}

float Taskbar::DrawTray(Window **windows, int windowCount)
{
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    float top = window->Pos.y;
    float bottom = top + THEME_TASKBAR_HEIGHT;
    float right = window->Pos.x + window->Size.x;

    ImVec2 stripMin(right - SHOW_DESKTOP, top + 1);
    ImGui::SetCursorScreenPos(stripMin);
    if(ImGui::InvisibleButton("##showDesktop", ImVec2(SHOW_DESKTOP, THEME_TASKBAR_HEIGHT - 1)))
        for(int i = 0; i < windowCount; i++)
            if(windows[i]->IsOpen())
                windows[i]->Minimize();

    Theme_ShowDesktop(stripMin, ImVec2(right, bottom), ImGui::IsItemHovered());

    //TODO: impl rtc service
    int seconds = (int)ImGui::GetTime();
    char clock[16];
    snprintf(clock, sizeof(clock), "%d:%02d:%02d", seconds / 3600, (seconds / 60) % 60, seconds % 60);

    ImVec2 size = ImGui::CalcTextSize(clock);
    float clockLeft = stripMin.x - CLOCK_WIDTH;
    ImVec2 pos(clockLeft + (CLOCK_WIDTH - size.x) * 0.5f, top + (THEME_TASKBAR_HEIGHT - size.y) * 0.5f);
    Theme_ShadowText(pos, clock);
    return clockLeft;
}

void Taskbar::Draw(Window **windows, int windowCount, StartMenu *startMenu)
{
    ImVec2 screen = ImGui::GetIO().DisplaySize;
    ImVec2 pos(0, screen.y - THEME_TASKBAR_HEIGHT);
    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(ImVec2(screen.x, THEME_TASKBAR_HEIGHT));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("##taskbar", nullptr, flags);
    ImGui::PopStyleVar();
    ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());

    Theme_Taskbar(pos, pos + ImVec2(screen.x, THEME_TASKBAR_HEIGHT));

    float trayLeft = DrawTray(windows, windowCount);
    DrawWindowButtons(windows, windowCount, trayLeft - 4);
    DrawOrb(startMenu);

    ImGui::End();
}
