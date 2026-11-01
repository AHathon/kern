#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include "backend/imgui/imgui.h"
#include "backend/imgui/imgui_internal.h"

#define THEME_FONT_SIZE             13.0f
#define THEME_TASKBAR_HEIGHT        32.0f
#define THEME_WINDOW_FRAME          4.0f
#define THEME_WINDOW_CAPTION        26.0f
#define THEME_WINDOW_ROUNDING       6.0f
#define THEME_CAPTION_BUTTONS_WIDTH 101.0f

#define THEME_COLOR_TEXT            IM_COL32(0, 0, 0, 255)
#define THEME_COLOR_TEXT_DETAIL     IM_COL32(90, 90, 90, 255)
#define THEME_COLOR_TEXT_DISABLED   IM_COL32(109, 109, 109, 255)
#define THEME_COLOR_HEADER_TEXT     IM_COL32(76, 96, 122, 255)
#define THEME_COLOR_HEADER_LINE     IM_COL32(222, 223, 234, 255)
#define THEME_COLOR_PANE            IM_COL32(255, 255, 255, 255)
#define THEME_COLOR_PANE_BORDER     IM_COL32(120, 145, 175, 255)

typedef enum
{
    THEME_CAPTION_NONE,
    THEME_CAPTION_MINIMIZE,
    THEME_CAPTION_MAXIMIZE,
    THEME_CAPTION_CLOSE
} ThemeCaptionButton;

void Theme_Apply();

void Theme_ShadowText(ImVec2 pos, const char *text, const ImVec4 *clip = nullptr);

bool Theme_Button(const char *label, ImVec2 size = ImVec2(0, 0), bool enabled = true);
void Theme_ProgressBar(float fraction);
bool Theme_Slider(const char *id, float *value, float min, float max);

void Theme_WindowFrame(ImVec2 min, ImVec2 max, const char *title, bool focused, bool maximized);
ThemeCaptionButton Theme_CaptionButtons(float right, float top, bool focused);

void Theme_Taskbar(ImVec2 min, ImVec2 max);
void Theme_TaskButton(ImVec2 min, ImVec2 max, const char *title, bool hovered, bool active);
void Theme_ShowDesktop(ImVec2 min, ImVec2 max, bool hovered);

void Theme_StartMenuFrame(ImVec2 min, ImVec2 max);
void Theme_StartMenuRow(ImVec2 min, ImVec2 max, const char *label, bool hovered, bool program);
void Theme_StartMenuSearch(ImVec2 min, ImVec2 max);
void Theme_StartMenuPlaces(ImVec2 min, ImVec2 max, const char *title);

void Theme_Toolbar(ImVec2 min, ImVec2 max);
void Theme_AddressBar(ImVec2 min, ImVec2 max);
void Theme_Crumb(ImVec2 min, ImVec2 max, const char *label, bool hovered);
void Theme_CrumbSeparator(ImVec2 pos);
void Theme_ListRow(ImVec2 min, ImVec2 max, bool selected, bool hovered);
void Theme_StatusBar(ImVec2 min, ImVec2 max, const char *text);
