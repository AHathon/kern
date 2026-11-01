#include "theme.h"

#define CAPTION_BUTTON_HEIGHT   18.0f
#define CAPTION_MIN_WIDTH       28.0f
#define CAPTION_MAX_WIDTH       27.0f
#define CAPTION_CLOSE_WIDTH     (THEME_CAPTION_BUTTONS_WIDTH - CAPTION_MIN_WIDTH - CAPTION_MAX_WIDTH)
#define MENU_ROUNDING           6.0f

#define COLOR_ROW_BORDER        IM_COL32(125, 162, 206, 255)
#define COLOR_PANEL_LINE        IM_COL32(190, 205, 222, 255)

typedef struct
{
    ImU32 top, midTop, midBottom, bottom;
} GlossColors;

typedef struct
{
    GlossColors normal, hovered, held;
} ButtonGloss;

static const ButtonGloss pushGloss =
{
    { IM_COL32(242, 242, 242, 255), IM_COL32(235, 235, 235, 255), IM_COL32(221, 221, 221, 255), IM_COL32(207, 207, 207, 255) },
    { IM_COL32(234, 246, 253, 255), IM_COL32(217, 240, 252, 255), IM_COL32(190, 230, 253, 255), IM_COL32(167, 217, 245, 255) },
    { IM_COL32(229, 244, 252, 255), IM_COL32(196, 229, 246, 255), IM_COL32(152, 209, 239, 255), IM_COL32(104, 179, 219, 255) }
};

static const ButtonGloss captionGloss =
{
    { IM_COL32(236, 243, 250, 255), IM_COL32(214, 227, 241, 255), IM_COL32(190, 208, 228, 255), IM_COL32(208, 222, 238, 255) },
    { IM_COL32(235, 245, 255, 255), IM_COL32(205, 228, 250, 255), IM_COL32(155, 200, 240, 255), IM_COL32(190, 225, 250, 255) },
    { IM_COL32(170, 200, 230, 255), IM_COL32(130, 170, 210, 255), IM_COL32(90, 135, 185, 255), IM_COL32(130, 170, 210, 255) }
};

static const ButtonGloss closeGloss =
{
    { IM_COL32(233, 170, 150, 255), IM_COL32(212, 96, 72, 255), IM_COL32(190, 45, 28, 255), IM_COL32(215, 110, 80, 255) },
    { IM_COL32(250, 190, 170, 255), IM_COL32(235, 110, 85, 255), IM_COL32(220, 50, 30, 255), IM_COL32(245, 150, 110, 255) },
    { IM_COL32(205, 120, 100, 255), IM_COL32(170, 50, 30, 255), IM_COL32(140, 20, 10, 255), IM_COL32(190, 80, 50, 255) }
};

static const GlossColors taskbarGloss = { IM_COL32(120, 168, 218, 255), IM_COL32(84, 136, 196, 255), IM_COL32(52, 104, 168, 255), IM_COL32(68, 122, 186, 255) };
static const GlossColors progressGloss = { IM_COL32(190, 245, 190, 255), IM_COL32(112, 229, 122, 255), IM_COL32(32, 196, 58, 255), IM_COL32(70, 214, 88, 255) };

void Theme_Apply()
{
    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(10, 10);
    style.WindowBorderSize = 0.0f;
    style.WindowRounding = 0.0f;
    style.WindowMinSize = ImVec2(170, 90);
    style.ChildBorderSize = 0.0f;
    style.FramePadding = ImVec2(5, 3);
    style.FrameBorderSize = 1.0f;
    style.FrameRounding = 0.0f;
    style.ItemSpacing = ImVec2(7, 7);
    style.ScrollbarSize = 16.0f;
    style.ScrollbarRounding = 0.0f;
    style.GrabRounding = 2.0f;

    ImVec4 *colors = style.Colors;
    colors[ImGuiCol_Text]              = ImColor(THEME_COLOR_TEXT);
    colors[ImGuiCol_TextDisabled]      = ImColor(THEME_COLOR_TEXT_DISABLED);
    colors[ImGuiCol_WindowBg]          = ImColor(240, 240, 240);
    colors[ImGuiCol_ChildBg]           = ImColor(240, 240, 240);
    colors[ImGuiCol_PopupBg]           = ImColor(240, 240, 240);
    colors[ImGuiCol_Border]            = ImColor(171, 173, 179);
    colors[ImGuiCol_FrameBg]           = ImColor(255, 255, 255);
    colors[ImGuiCol_FrameBgHovered]    = ImColor(255, 255, 255);
    colors[ImGuiCol_FrameBgActive]     = ImColor(255, 255, 255);
    colors[ImGuiCol_ScrollbarBg]       = ImColor(240, 240, 240);
    colors[ImGuiCol_ScrollbarGrab]     = ImColor(205, 205, 205);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(190, 230, 253);
    colors[ImGuiCol_ScrollbarGrabActive]  = ImColor(150, 205, 240);
    colors[ImGuiCol_CheckMark]         = ImColor(74, 95, 151);
    colors[ImGuiCol_SliderGrab]        = ImColor(205, 205, 205);
    colors[ImGuiCol_SliderGrabActive]  = ImColor(150, 205, 240);
    colors[ImGuiCol_Button]            = ImColor(225, 225, 225);
    colors[ImGuiCol_ButtonHovered]     = ImColor(190, 230, 253);
    colors[ImGuiCol_ButtonActive]      = ImColor(150, 205, 240);
    colors[ImGuiCol_Header]            = ImColor(207, 228, 250);
    colors[ImGuiCol_HeaderHovered]     = ImColor(217, 235, 252);
    colors[ImGuiCol_HeaderActive]      = ImColor(190, 220, 250);
    colors[ImGuiCol_Separator]         = ImColor(213, 223, 229);
    colors[ImGuiCol_TextSelectedBg]    = ImColor(51, 153, 255, 120);
    colors[ImGuiCol_InputTextCursor]   = ImColor(0, 0, 0);

    colors[ImGuiCol_ResizeGrip]        = ImColor(0, 0, 0, 0);
    colors[ImGuiCol_ResizeGripHovered] = ImColor(0, 0, 0, 0);
    colors[ImGuiCol_ResizeGripActive]  = ImColor(0, 0, 0, 0);
}

static void Shade(ImDrawList *list, int firstVertex, float top, float bottom, ImU32 topColor, ImU32 bottomColor)
{
    for(int i = firstVertex; i < list->VtxBuffer.Size; i++)
    {
        ImDrawVert &vert = list->VtxBuffer[i];
        float t = ImSaturate((vert.pos.y - top) / (bottom - top));

        ImU32 color = 0;
        for(int shift = 0; shift < 32; shift += 8)
        {
            float a = (float)((topColor >> shift) & 0xFF);
            float b = (float)((bottomColor >> shift) & 0xFF);
            color |= (ImU32)(a + (b - a) * t + 0.5f) << shift;
        }

        ImU32 alpha = ((color >> IM_COL32_A_SHIFT) & 0xFF) * ((vert.col >> IM_COL32_A_SHIFT) & 0xFF) / 255;
        vert.col = (color & ~IM_COL32_A_MASK) | (alpha << IM_COL32_A_SHIFT);
    }
}

static void Gradient(ImDrawList *list, ImVec2 min, ImVec2 max, ImU32 top, ImU32 bottom, float rounding = 0.0f, ImDrawFlags flags = 0)
{
    if(rounding <= 0.0f && (top & IM_COL32_A_MASK) == IM_COL32_A_MASK && (bottom & IM_COL32_A_MASK) == IM_COL32_A_MASK)
    {
        ImVec4 a = ImGui::ColorConvertU32ToFloat4(top);
        ImVec4 b = ImGui::ColorConvertU32ToFloat4(bottom);
        for(float y = min.y; y < max.y; y += 1.0f)
        {
            float t = (y + 0.5f - min.y) / (max.y - min.y);
            list->AddRectFilled(ImVec2(min.x, y), ImVec2(max.x, ImMin(y + 1.0f, max.y)), ImGui::ColorConvertFloat4ToU32(ImLerp(a, b, t)));
        }
        return;
    }

    int first = list->VtxBuffer.Size;
    list->AddRectFilled(min, max, IM_COL32_WHITE, rounding, flags);
    Shade(list, first, min.y, max.y, top, bottom);
}

static void Gloss(ImDrawList *list, ImVec2 min, ImVec2 max, const GlossColors &colors, float rounding = 0.0f, ImDrawFlags flags = 0)
{
    float mid = IM_ROUND((min.y + max.y) * 0.5f);
    ImDrawFlags topFlags = flags == 0 ? ImDrawFlags_RoundTop : flags & ImDrawFlags_RoundTop;
    ImDrawFlags bottomFlags = flags == 0 ? ImDrawFlags_RoundBottom : flags & ImDrawFlags_RoundBottom;

    Gradient(list, min, ImVec2(max.x, mid), colors.top, colors.midTop, topFlags ? rounding : 0.0f, topFlags ? topFlags : ImDrawFlags_RoundNone);
    Gradient(list, ImVec2(min.x, mid), max, colors.midBottom, colors.bottom, bottomFlags ? rounding : 0.0f, bottomFlags ? bottomFlags : ImDrawFlags_RoundNone);
}

static const GlossColors &PickGloss(const ButtonGloss &gloss, bool hovered, bool held)
{
    return held ? gloss.held : (hovered ? gloss.hovered : gloss.normal);
}

static void WindowIcon(ImDrawList *list, ImVec2 pos)
{
    list->AddRectFilled(pos, pos + ImVec2(14, 12), IM_COL32(255, 255, 255, 255), 1.0f);
    list->AddRectFilled(pos, pos + ImVec2(14, 4), IM_COL32(58, 118, 190, 255), 1.0f, ImDrawFlags_RoundTop);
    list->AddRect(pos, pos + ImVec2(14, 12), IM_COL32(32, 66, 110, 255), 1.0f);
}

void Theme_ShadowText(ImVec2 pos, const char *text, const ImVec4 *clip)
{
    ImDrawList *list = ImGui::GetWindowDrawList();
    ImFont *font = ImGui::GetFont();
    list->AddText(font, THEME_FONT_SIZE, pos + ImVec2(0, 1), IM_COL32(0, 0, 0, 120), text, nullptr, 0.0f, clip);
    list->AddText(font, THEME_FONT_SIZE, pos, IM_COL32_WHITE, text, nullptr, 0.0f, clip);
}

bool Theme_Button(const char *label, ImVec2 size, bool enabled)
{
    ImVec2 textSize = ImGui::CalcTextSize(label, nullptr, true);
    if(size.x <= 0.0f)
        size.x = ImMax(textSize.x + 22.0f, 75.0f);
    if(size.y <= 0.0f)
        size.y = 23.0f;

    if(!enabled)
        ImGui::BeginDisabled();
    bool pressed = ImGui::InvisibleButton(label, size);
    if(!enabled)
        ImGui::EndDisabled();

    bool hovered = ImGui::IsItemHovered() && enabled;
    bool held = ImGui::IsItemActive();

    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();
    ImDrawList *list = ImGui::GetWindowDrawList();

    ImU32 border = IM_COL32(112, 112, 112, 255);
    if(!enabled)
    {
        Gradient(list, min, max, IM_COL32(244, 244, 244, 255), IM_COL32(244, 244, 244, 255), 3.0f);
        border = IM_COL32(173, 178, 181, 255);
    }
    else
    {
        Gloss(list, min, max, PickGloss(pushGloss, hovered, held), 3.0f);
        if(held)
            border = IM_COL32(44, 98, 139, 255);
        else if(hovered)
            border = IM_COL32(60, 127, 177, 255);
    }

    list->AddRect(min + ImVec2(1, 1), max - ImVec2(1, 1), IM_COL32(255, 255, 255, 170), 2.0f);
    list->AddRect(min, max, border, 3.0f);

    ImU32 text = ImGui::GetColorU32(enabled ? ImGuiCol_Text : ImGuiCol_TextDisabled);
    list->AddText(min + (size - textSize) * 0.5f, text, label, ImGui::FindRenderedTextEnd(label));
    return pressed;
}

void Theme_ProgressBar(float fraction)
{
    ImVec2 size(ImGui::GetContentRegionAvail().x, 16.0f);
    ImVec2 min = ImGui::GetCursorScreenPos();
    ImVec2 max = min + size;
    ImGui::Dummy(size);

    ImDrawList *list = ImGui::GetWindowDrawList();
    Gradient(list, min, max, IM_COL32(222, 222, 222, 255), IM_COL32(244, 244, 244, 255), 2.0f);

    float fill = (float)(int)((size.x - 2.0f) * ImSaturate(fraction));
    if(fill > 0.0f)
        Gloss(list, min + ImVec2(1, 1), ImVec2(min.x + 1.0f + fill, max.y - 1.0f), progressGloss);

    list->AddRect(min, max, IM_COL32(150, 150, 150, 255), 2.0f);
}

bool Theme_Slider(const char *id, float *value, float min, float max)
{
    const ImVec2 thumb(11, 19);
    ImVec2 size(ImGui::GetContentRegionAvail().x, thumb.y + 2.0f);

    ImGui::InvisibleButton(id, size);
    bool hovered = ImGui::IsItemHovered();
    bool held = ImGui::IsItemActive();
    ImVec2 itemMin = ImGui::GetItemRectMin();

    float travel = size.x - thumb.x;
    bool changed = false;
    if(held && travel > 0.0f)
    {
        float t = ImSaturate((ImGui::GetIO().MousePos.x - itemMin.x - thumb.x * 0.5f) / travel);
        *value = min + (max - min) * t;
        changed = true;
    }

    ImDrawList *list = ImGui::GetWindowDrawList();
    float centerY = itemMin.y + (float)(int)(size.y * 0.5f);
    list->AddRectFilled(ImVec2(itemMin.x, centerY - 2), ImVec2(itemMin.x + size.x, centerY + 2), IM_COL32(231, 234, 234, 255));
    list->AddRect(ImVec2(itemMin.x, centerY - 2), ImVec2(itemMin.x + size.x, centerY + 2), IM_COL32(176, 176, 176, 255));

    float t = max > min ? ImSaturate((*value - min) / (max - min)) : 0.0f;
    ImVec2 thumbMin(itemMin.x + (float)(int)(travel * t), itemMin.y + 1.0f);
    ImVec2 thumbMax = thumbMin + thumb;
    if(held)
        Gradient(list, thumbMin, thumbMax, IM_COL32(196, 229, 246, 255), IM_COL32(104, 179, 219, 255), 2.0f);
    else if(hovered)
        Gradient(list, thumbMin, thumbMax, IM_COL32(234, 246, 253, 255), IM_COL32(167, 217, 245, 255), 2.0f);
    else
        Gradient(list, thumbMin, thumbMax, IM_COL32(245, 245, 245, 255), IM_COL32(208, 208, 208, 255), 2.0f);
    list->AddRect(thumbMin, thumbMax, hovered || held ? IM_COL32(60, 127, 177, 255) : IM_COL32(112, 112, 112, 255), 2.0f);
    return changed;
}

void Theme_WindowFrame(ImVec2 min, ImVec2 max, const char *title, bool focused, bool maximized)
{
    ImDrawList *list = ImGui::GetWindowDrawList();
    float rounding = maximized ? 0.0f : THEME_WINDOW_ROUNDING;

    list->PushClipRectFullScreen();
    if(!maximized)
        for(int i = 1; i <= 4; i++)
            list->AddRect(min - ImVec2((float)i, (float)i), max + ImVec2((float)i, (float)i), IM_COL32(0, 0, 0, 30 - i * 6), rounding + i);
    list->PopClipRect();

    float captionBottom = min.y + THEME_WINDOW_CAPTION;
    ImU32 edge = focused ? IM_COL32(205, 222, 240, 255) : IM_COL32(232, 239, 247, 255);
    Gradient(list, min, ImVec2(max.x, captionBottom), focused ? IM_COL32(160, 192, 226, 255) : IM_COL32(215, 228, 242, 255), edge, rounding, ImDrawFlags_RoundTop);
    list->AddRectFilled(ImVec2(min.x, captionBottom), ImVec2(min.x + THEME_WINDOW_FRAME, max.y), edge);
    list->AddRectFilled(ImVec2(max.x - THEME_WINDOW_FRAME, captionBottom), max, edge);
    list->AddRectFilled(ImVec2(min.x + THEME_WINDOW_FRAME, max.y - THEME_WINDOW_FRAME), ImVec2(max.x - THEME_WINDOW_FRAME, max.y), edge);

    Gradient(list, min + ImVec2(1, 1), ImVec2(max.x - 1, min.y + THEME_WINDOW_CAPTION * 0.5f), IM_COL32(255, 255, 255, focused ? 150 : 90), IM_COL32(255, 255, 255, 0), rounding, ImDrawFlags_RoundTop);

    list->AddRect(min, max, focused ? IM_COL32(52, 82, 120, 255) : IM_COL32(120, 140, 165, 255), rounding, 1.0f, ImDrawFlags_RoundTop);
    list->AddRect(ImVec2(min.x + THEME_WINDOW_FRAME - 1, captionBottom - 1), ImVec2(max.x - THEME_WINDOW_FRAME + 1, max.y - THEME_WINDOW_FRAME + 1), THEME_COLOR_PANE_BORDER);

    WindowIcon(list, min + ImVec2(7, 6));
    list->AddText(min + ImVec2(26, 5), focused ? THEME_COLOR_TEXT : THEME_COLOR_TEXT_DETAIL, title);
}

static bool CaptionButton(const char *id, ImVec2 min, float width, bool close, bool focused, ImDrawFlags corners)
{
    ImGui::SetCursorScreenPos(min);
    bool pressed = ImGui::InvisibleButton(id, ImVec2(width, CAPTION_BUTTON_HEIGHT));
    bool hovered = ImGui::IsItemHovered();
    bool held = ImGui::IsItemActive();

    ImDrawList *list = ImGui::GetWindowDrawList();
    ImVec2 max = min + ImVec2(width, CAPTION_BUTTON_HEIGHT);

    const ButtonGloss &gloss = close && (focused || hovered) ? closeGloss : captionGloss;
    Gloss(list, min, max, PickGloss(gloss, hovered, held), 3.0f, corners);

    list->AddRect(min, max, focused ? IM_COL32(60, 85, 120, 255) : IM_COL32(135, 150, 170, 255), 3.0f, 1.0f, corners);
    return pressed;
}

ThemeCaptionButton Theme_CaptionButtons(float right, float top, bool focused)
{
    ImDrawList *list = ImGui::GetWindowDrawList();
    ImU32 glyph = focused ? IM_COL32(32, 44, 64, 255) : IM_COL32(110, 120, 135, 255);
    ThemeCaptionButton pressed = THEME_CAPTION_NONE;

    ImVec2 closeMin(right - CAPTION_CLOSE_WIDTH, top);
    ImVec2 maxMin = closeMin - ImVec2(CAPTION_MAX_WIDTH, 0);
    ImVec2 minMin = maxMin - ImVec2(CAPTION_MIN_WIDTH, 0);

    if(CaptionButton("##min", minMin, CAPTION_MIN_WIDTH, false, focused, ImDrawFlags_RoundBottomLeft))
        pressed = THEME_CAPTION_MINIMIZE;
    list->AddRectFilled(minMin + ImVec2(9, 11), minMin + ImVec2(19, 14), glyph);

    if(CaptionButton("##max", maxMin, CAPTION_MAX_WIDTH, false, focused, ImDrawFlags_RoundNone))
        pressed = THEME_CAPTION_MAXIMIZE;
    list->AddRect(maxMin + ImVec2(9, 5), maxMin + ImVec2(18, 13), glyph);
    list->AddRectFilled(maxMin + ImVec2(9, 5), maxMin + ImVec2(18, 7), glyph);

    if(CaptionButton("##close", closeMin, CAPTION_CLOSE_WIDTH, true, focused, ImDrawFlags_RoundBottomRight))
        pressed = THEME_CAPTION_CLOSE;
    ImVec2 center = closeMin + ImVec2(CAPTION_CLOSE_WIDTH * 0.5f, CAPTION_BUTTON_HEIGHT * 0.5f);
    ImU32 cross = focused ? IM_COL32(255, 255, 255, 255) : glyph;
    list->AddLine(center + ImVec2(-4, -4), center + ImVec2(4, 4), cross, 2.0f);
    list->AddLine(center + ImVec2(-4, 4), center + ImVec2(4, -4), cross, 2.0f);
    return pressed;
}

void Theme_Taskbar(ImVec2 min, ImVec2 max)
{
    ImDrawList *list = ImGui::GetWindowDrawList();
    Gloss(list, min, max, taskbarGloss);
    list->AddLine(min, ImVec2(max.x, min.y), IM_COL32(18, 48, 92, 255));
    list->AddLine(min + ImVec2(0, 1), ImVec2(max.x, min.y + 1), IM_COL32(190, 216, 244, 200));
}

void Theme_TaskButton(ImVec2 min, ImVec2 max, const char *title, bool hovered, bool active)
{
    ImDrawList *list = ImGui::GetWindowDrawList();
    int glow = (active ? 70 : 34) + (hovered ? 34 : 0);
    GlossColors colors = { IM_COL32(255, 255, 255, glow + 26), IM_COL32(255, 255, 255, glow), IM_COL32(255, 255, 255, glow - 20), IM_COL32(255, 255, 255, glow) };
    Gloss(list, min, max, colors, 3.0f);
    list->AddRect(min, max, IM_COL32(255, 255, 255, active ? 170 : 90), 3.0f);
    list->AddRect(min - ImVec2(1, 1), max + ImVec2(1, 1), IM_COL32(10, 34, 70, 120), 4.0f);

    WindowIcon(list, min + ImVec2(7, 8));

    ImVec4 clip(min.x + 26, min.y, max.x - 5, max.y);
    Theme_ShadowText(ImVec2(clip.x, min.y + 7), title, &clip);
}

void Theme_ShowDesktop(ImVec2 min, ImVec2 max, bool hovered)
{
    ImDrawList *list = ImGui::GetWindowDrawList();
    if(hovered)
        list->AddRectFilled(min, max, IM_COL32(255, 255, 255, 60));
    list->AddLine(min, ImVec2(min.x, max.y), IM_COL32(10, 34, 70, 150));
    list->AddLine(min + ImVec2(1, 0), ImVec2(min.x + 1, max.y), IM_COL32(255, 255, 255, 60));
}

void Theme_StartMenuFrame(ImVec2 min, ImVec2 max)
{
    ImDrawList *list = ImGui::GetWindowDrawList();
    list->AddRectFilled(min, max, IM_COL32(205, 222, 240, 255), MENU_ROUNDING, ImDrawFlags_RoundTop);
    list->AddRect(min, max, IM_COL32(52, 82, 120, 255), MENU_ROUNDING, 1.0f, ImDrawFlags_RoundTop);
}

void Theme_StartMenuRow(ImVec2 min, ImVec2 max, const char *label, bool hovered, bool program)
{
    ImDrawList *list = ImGui::GetWindowDrawList();
    if(hovered)
    {
        Gradient(list, min, max, IM_COL32(236, 245, 254, 90), IM_COL32(190, 220, 250, 150), 3.0f);
        list->AddRect(min, max, COLOR_ROW_BORDER, 3.0f);
    }

    float textX = min.x + 10;
    if(program)
    {
        WindowIcon(list, min + ImVec2(8, 7));
        textX += 21;
    }
    list->AddText(ImVec2(textX, min.y + 5), program ? THEME_COLOR_TEXT : IM_COL32_WHITE, label);
}

void Theme_StartMenuSearch(ImVec2 min, ImVec2 max)
{
    ImDrawList *list = ImGui::GetWindowDrawList();
    list->AddRectFilled(min, max, IM_COL32(233, 239, 246, 255));
    list->AddLine(min, ImVec2(max.x, min.y), COLOR_PANEL_LINE);
}

void Theme_StartMenuPlaces(ImVec2 min, ImVec2 max, const char *title)
{
    ImDrawList *list = ImGui::GetWindowDrawList();
    Gradient(list, min, max, IM_COL32(32, 86, 152, 255), IM_COL32(20, 54, 104, 255));
    list->AddText(min + ImVec2(14, 13), IM_COL32_WHITE, title);
    list->AddLine(min + ImVec2(12, 36), ImVec2(max.x - 12, min.y + 36), IM_COL32(255, 255, 255, 60));
}

void Theme_Toolbar(ImVec2 min, ImVec2 max)
{
    ImDrawList *list = ImGui::GetWindowDrawList();
    Gradient(list, min, max, IM_COL32(245, 250, 254, 255), IM_COL32(217, 230, 243, 255));
    list->AddLine(ImVec2(min.x, max.y - 1), ImVec2(max.x, max.y - 1), IM_COL32(175, 192, 212, 255));
}

void Theme_AddressBar(ImVec2 min, ImVec2 max)
{
    ImDrawList *list = ImGui::GetWindowDrawList();
    list->AddRectFilled(min, max, THEME_COLOR_PANE);
    list->AddRect(min, max, IM_COL32(171, 173, 179, 255));
}

void Theme_Crumb(ImVec2 min, ImVec2 max, const char *label, bool hovered)
{
    ImDrawList *list = ImGui::GetWindowDrawList();
    if(hovered)
    {
        Gradient(list, min, max, IM_COL32(236, 245, 254, 255), IM_COL32(205, 228, 250, 255));
        list->AddRect(min, max, COLOR_ROW_BORDER);
    }
    list->AddText(min + ImVec2(5, 3), THEME_COLOR_TEXT, label);
}

void Theme_CrumbSeparator(ImVec2 pos)
{
    ImGui::GetWindowDrawList()->AddTriangleFilled(pos + ImVec2(4, 0), pos + ImVec2(0, 4), pos - ImVec2(0, 4), IM_COL32(90, 100, 115, 255));
}

void Theme_ListRow(ImVec2 min, ImVec2 max, bool selected, bool hovered)
{
    ImDrawList *list = ImGui::GetWindowDrawList();
    if(selected)
    {
        Gradient(list, min, max, IM_COL32(220, 235, 252, 255), IM_COL32(193, 219, 252, 255), 2.0f);
        list->AddRect(min, max, COLOR_ROW_BORDER, 2.0f);
    }
    else if(hovered)
    {
        Gradient(list, min, max, IM_COL32(250, 251, 253, 255), IM_COL32(235, 243, 253, 255), 2.0f);
        list->AddRect(min, max, IM_COL32(184, 214, 251, 255), 2.0f);
    }
}

void Theme_StatusBar(ImVec2 min, ImVec2 max, const char *text)
{
    ImDrawList *list = ImGui::GetWindowDrawList();
    Gradient(list, min, max, IM_COL32(245, 249, 253, 255), IM_COL32(225, 234, 245, 255));
    list->AddLine(min, ImVec2(max.x, min.y), COLOR_PANEL_LINE);
    list->AddText(min + ImVec2(8, 5), IM_COL32(40, 50, 65, 255), text);
}
