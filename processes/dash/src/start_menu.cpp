#include "theme.h"
#include "start_menu.h"

#define MENU_WIDTH      340.0f
#define MENU_HEIGHT     330.0f
#define MENU_FRAME      5.0f
#define LEFT_PANE_WIDTH 210.0f
#define ROW_HEIGHT      28.0f
#define SEARCH_HEIGHT   40.0f

void StartMenu::AddProgram(Window *window)
{
    if(programCount < START_MENU_MAX_ENTRIES)
        programs[programCount++] = window;
}

void StartMenu::AddPlace(Window *window)
{
    if(placeCount < START_MENU_MAX_ENTRIES)
        places[placeCount++] = window;
}

void StartMenu::Toggle()
{
    open = !open;
    focusSearch = open;
    search[0] = '\0';
}

static bool Row(const char *label, ImVec2 min, float width, bool program)
{
    ImVec2 size(width, ROW_HEIGHT - 2);

    ImGui::SetCursorScreenPos(min);
    ImGui::PushID(label);
    bool pressed = ImGui::InvisibleButton("##row", size);
    ImGui::PopID();

    Theme_StartMenuRow(min, min + size, label, ImGui::IsItemHovered(), program);
    return pressed;
}

void StartMenu::DrawPrograms(float x0, float y0, float x1, float y1)
{
    ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), THEME_COLOR_PANE);

    float rowY = y0 + 6;
    Window *first = nullptr;
    for(int i = 0; i < programCount; i++)
    {
        if(search[0] && !ImStristr(programs[i]->Title(), nullptr, search, nullptr))
            continue;
        if(!first)
            first = programs[i];

        if(Row(programs[i]->Title(), ImVec2(x0 + 4, rowY), x1 - x0 - 8, true))
        {
            programs[i]->Open();
            Close();
        }
        rowY += ROW_HEIGHT;
    }

    float searchTop = y1 - SEARCH_HEIGHT;
    Theme_StartMenuSearch(ImVec2(x0, searchTop), ImVec2(x1, y1));

    ImGui::SetCursorScreenPos(ImVec2(x0 + 8, searchTop + 9));
    ImGui::SetNextItemWidth(x1 - x0 - 16);
    if(focusSearch)
    {
        ImGui::SetKeyboardFocusHere();
        focusSearch = false;
    }
    if(ImGui::InputTextWithHint("##search", "Search programs", search, sizeof(search), ImGuiInputTextFlags_EnterReturnsTrue) && first)
    {
        first->Open();
        Close();
    }
}

void StartMenu::DrawPlaces(float x0, float y0, float x1, float y1)
{
    Theme_StartMenuPlaces(ImVec2(x0, y0), ImVec2(x1, y1), "kern");

    float rowY = y0 + 42;
    for(int i = 0; i < placeCount; i++)
    {
        if(Row(places[i]->Title(), ImVec2(x0 + 6, rowY), x1 - x0 - 12, false))
        {
            places[i]->Open();
            Close();
        }
        rowY += ROW_HEIGHT;
    }

    //TODO shutdown
    ImGui::SetCursorScreenPos(ImVec2(x0 + 12, y1 - 32));
    Theme_Button("Shut down", ImVec2(x1 - x0 - 24, 24), false);
}

void StartMenu::Draw(bool orbHovered)
{
    if(!open)
        return;

    ImVec2 screen = ImGui::GetIO().DisplaySize;
    ImVec2 pos(0, screen.y - THEME_TASKBAR_HEIGHT - MENU_HEIGHT);
    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(ImVec2(MENU_WIDTH, MENU_HEIGHT));
    ImGui::SetNextWindowFocus();

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("##startMenu", nullptr, flags);
    ImGui::PopStyleVar();
    ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());

    ImVec2 max = pos + ImVec2(MENU_WIDTH, MENU_HEIGHT);
    Theme_StartMenuFrame(pos, max);

    float left = pos.x + MENU_FRAME;
    float top = pos.y + MENU_FRAME;
    float split = left + LEFT_PANE_WIDTH;
    float right = max.x - MENU_FRAME;
    float bottom = max.y - MENU_FRAME;

    DrawPrograms(left, top, split, bottom);
    DrawPlaces(split, top, right, bottom);
    ImGui::GetWindowDrawList()->AddRect(ImVec2(left - 1, top - 1), ImVec2(right + 1, bottom + 1), THEME_COLOR_PANE_BORDER);

    bool outside =ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !orbHovered && !ImGui::IsMouseHoveringRect(pos, max, false);
    if(outside || ImGui::IsKeyPressed(ImGuiKey_Escape))
        Close();

    ImGui::End();
}
