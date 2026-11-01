#include "theme.h"
#include "file_browser.h"
#include "services.h"
#include "libraries/libc/stdio.h"
#include "libraries/libc/string.h"

#define TOOLBAR_HEIGHT  36.0f
#define HEADER_HEIGHT   22.0f
#define STATUS_HEIGHT   24.0f
#define ROW_HEIGHT      21.0f
#define SIZE_WIDTH      80.0f
#define TYPE_WIDTH      110.0f
#define ROW_PADDING     6.0f

static bool IsDirectory(const FsDirEntry *entry)
{
    return entry->attr & FS_ATTR_DIRECTORY;
}

static bool SortsBefore(const FsDirEntry *a, const FsDirEntry *b)
{
    if(IsDirectory(a) != IsDirectory(b))
        return IsDirectory(a);
    return strcmp(a->name, b->name) < 0;
}

static void FormatSize(char *out, size_t size, uint64_t bytes)
{
    snprintf(out, size, "%lu KB", (bytes / 1024) + (bytes % 1024 ? 1 : 0));
}

static void FormatType(char *out, size_t size, const FsDirEntry *entry)
{
    if(IsDirectory(entry))
    {
        snprintf(out, size, "File folder");
        return;
    }

    const char *dot = strrchr(entry->name, '.');
    if(!dot)
    {
        snprintf(out, size, "File");
        return;
    }

    char extension[4] = "";
    for(int i = 0; i < 3 && dot[i + 1]; i++)
        extension[i] = ImToUpper(dot[i + 1]);
    snprintf(out, size, "%s File", extension);
}

bool FileBrowser::Load(const char *newPath)
{
    entryCount = 0;
    selected = -1;
    truncated = false;

    while(entryCount < FILE_BROWSER_MAX_ENTRIES)
    {
        uint32_t count = 0;
        error = fsListDir(&fs, newPath, entryCount, entries + entryCount, FILE_BROWSER_MAX_ENTRIES - entryCount, &count);
        if(error != FS_OK)
        {
            entryCount = 0;
            return false;
        }
        if(count == 0)
            break;
        entryCount += count;
    }

    if(entryCount == FILE_BROWSER_MAX_ENTRIES)
    {
        FsDirEntry extra;
        uint32_t count = 0;
        truncated = fsListDir(&fs, newPath, entryCount, &extra, 1, &count) == FS_OK && count > 0;
    }

    for(int i = 1; i < entryCount; i++)
    {
        FsDirEntry entry = entries[i];
        int j = i;
        for(; j > 0 && SortsBefore(&entry, &entries[j - 1]); j--)
            entries[j] = entries[j - 1];
        entries[j] = entry;
    }

    if(newPath != path)
        ImStrncpy(path, newPath, sizeof(path));
    return true;
}

void FileBrowser::Navigate(const char *newPath)
{
    if(!Load(newPath))
        return;

    if(historyPos == FILE_BROWSER_MAX_HISTORY - 1)
    {
        memmove(history[0], history[1], sizeof(history[0]) * (FILE_BROWSER_MAX_HISTORY - 1));
        historyPos--;
    }

    historyPos++;
    historyCount = historyPos + 1;
    ImStrncpy(history[historyPos], path, sizeof(history[historyPos]));
}

void FileBrowser::Back()
{
    if(historyPos > 0 && Load(history[historyPos - 1]))
        historyPos--;
}

void FileBrowser::Forward()
{
    if(historyPos + 1 < historyCount && Load(history[historyPos + 1]))
        historyPos++;
}

void FileBrowser::OpenEntry(const FsDirEntry *entry)
{
    if(!IsDirectory(entry))
        return;

    char next[FS_MAX_PATH];
    int length = snprintf(next, sizeof(next), "%s/%s", path[1] ? path : "", entry->name);
    if(length > 0 && length < (int)sizeof(next))
        Navigate(next);
}

static bool Crumb(const char *label, int depth, float *x, float y, float height)
{
    ImVec2 size(ImGui::CalcTextSize(label).x + 10.0f, height);
    ImVec2 min(*x, y);
    ImVec2 max = min + size;

    ImGui::SetCursorScreenPos(min);
    ImGui::PushID(depth);
    bool pressed = ImGui::InvisibleButton("##crumb", size);
    ImGui::PopID();

    Theme_Crumb(min, max, label, ImGui::IsItemHovered());

    *x += size.x;
    return pressed;
}

void FileBrowser::DrawAddress(float x0, float y0, float x1, float y1)
{
    ImDrawList *list = ImGui::GetWindowDrawList();
    ImVec2 min(x0, y0);
    ImVec2 max(x1, y1);
    Theme_AddressBar(min, max);
    list->PushClipRect(min, max, true);

    Theme_FolderIcon(min + ImVec2(5, 3));

    float x = x0 + 25.0f;
    float top = y0 + 1.0f;
    float height = y1 - y0 - 2.0f;
    const char *start = path;
    const char *end = path + 1;
    for(int depth = 0; ; depth++)
    {
        if(depth > 0)
        {
            Theme_CrumbSeparator(ImVec2(x + 3.0f, top + height * 0.5f));
            x += 11.0f;
        }

        char label[FS_MAX_PATH];
        ImStrncpy(label, start, end - start + 1);
        if(Crumb(label, depth, &x, top, height))
        {
            char target[FS_MAX_PATH];
            ImStrncpy(target, path, end - path + 1);
            Navigate(target);
            break;
        }

        start = *end == '/' ? end + 1 : end;
        if(!*start)
            break;

        end = strchr(start, '/');
        if(!end)
            end = start + strlen(start);
    }

    list->PopClipRect();
}

void FileBrowser::DrawToolbar(float x, float y, float width)
{
    Theme_Toolbar(ImVec2(x, y), ImVec2(x + width, y + TOOLBAR_HEIGHT));

    float centerY = y + TOOLBAR_HEIGHT * 0.5f;
    if(Theme_NavButton("##back", ImVec2(x + 18, centerY), -1.0f, historyPos > 0))
        Back();
    if(Theme_NavButton("##forward", ImVec2(x + 44, centerY), 1.0f, historyPos + 1 < historyCount))
        Forward();

    DrawAddress(x + 64, y + 6, x + width - 8, y + TOOLBAR_HEIGHT - 7);
}

void FileBrowser::DrawHeader(float x, float y, float width)
{
    ImDrawList *list = ImGui::GetWindowDrawList();
    const ImU32 text = THEME_COLOR_HEADER_TEXT;
    const ImU32 line = THEME_COLOR_HEADER_LINE;

    list->AddRectFilled(ImVec2(x, y), ImVec2(x + width, y + HEADER_HEIGHT), THEME_COLOR_PANE);
    list->AddLine(ImVec2(x, y + HEADER_HEIGHT - 1), ImVec2(x + width, y + HEADER_HEIGHT - 1), line);

    float typeX = x + width - TYPE_WIDTH;
    float sizeX = typeX - SIZE_WIDTH;
    float textY = y + 4;
    list->AddText(ImVec2(x + 28, textY), text, "Name");
    list->AddText(ImVec2(sizeX + 8, textY), text, "Size");
    list->AddText(ImVec2(typeX + 8, textY), text, "Type");
    list->AddLine(ImVec2(sizeX, y), ImVec2(sizeX, y + HEADER_HEIGHT - 1), line);
    list->AddLine(ImVec2(typeX, y), ImVec2(typeX, y + HEADER_HEIGHT - 1), line);
}

void FileBrowser::DrawList(float x, float y, float width, float height)
{
    ImGui::SetCursorScreenPos(ImVec2(x, y));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, THEME_COLOR_PANE);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::BeginChild("##files", ImVec2(width, ImMax(height, 1.0f)));
    ImGui::PopStyleVar();

    ImDrawList *list = ImGui::GetWindowDrawList();
    ImVec2 origin = ImGui::GetCursorScreenPos();
    if(error != FS_OK || entryCount == 0)
    {
        char message[64];
        if(error != FS_OK)
            snprintf(message, sizeof(message), "This folder can't be opened (error %ld)", error);
        else
            snprintf(message, sizeof(message), "This folder is empty.");
        list->AddText(origin + ImVec2(28, 8), THEME_COLOR_TEXT_DISABLED, message);
    }

    float rowWidth = ImGui::GetContentRegionAvail().x;
    float typeX = origin.x + rowWidth - TYPE_WIDTH;
    float sizeX = typeX - SIZE_WIDTH;
    int open = -1;

    ImGuiListClipper clipper;
    clipper.Begin(entryCount, ROW_HEIGHT);
    while(clipper.Step())
    {
        for(int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
        {
            const FsDirEntry *entry = &entries[i];

            ImGui::PushID(i);
            ImGui::InvisibleButton("##row", ImVec2(rowWidth, ROW_HEIGHT));
            ImGui::PopID();

            bool hovered = ImGui::IsItemHovered();
            if(ImGui::IsItemClicked())
                selected = i;
            if(hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                open = i;

            ImVec2 min = ImGui::GetItemRectMin();
            ImVec2 max = ImGui::GetItemRectMax() - ImVec2(0, 1);
            Theme_ListRow(min, max, i == selected, hovered);

            if(IsDirectory(entry))
                Theme_FolderIcon(min + ImVec2(ROW_PADDING, 2));
            else
                Theme_FileIcon(min + ImVec2(ROW_PADDING, 2));

            float textY = min.y + 3;
            list->PushClipRect(min, ImVec2(sizeX, max.y), true);
            list->AddText(ImVec2(min.x + 28, textY), THEME_COLOR_TEXT, entry->name);
            list->PopClipRect();

            char type[20];
            FormatType(type, sizeof(type), entry);
            list->AddText(ImVec2(typeX + 8, textY), THEME_COLOR_TEXT_DETAIL, type);

            if(!IsDirectory(entry))
            {
                char size[20];
                FormatSize(size, sizeof(size), entry->size);
                list->AddText(ImVec2(typeX - 8 - ImGui::CalcTextSize(size).x, textY), THEME_COLOR_TEXT_DETAIL, size);
            }
        }
    }

    if(ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        selected = -1;

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    if(open >= 0)
        OpenEntry(&entries[open]);
}

void FileBrowser::DrawStatus(float x, float y, float width)
{
    char status[64];
    if(selected >= 0)
    {
        char size[20];
        FormatSize(size, sizeof(size), entries[selected].size);
        snprintf(status, sizeof(status), "%s  %s", entries[selected].name, IsDirectory(&entries[selected]) ? "" : size);
    }
    else if(truncated)
        snprintf(status, sizeof(status), "Showing the first %d items", entryCount);
    else
        snprintf(status, sizeof(status), entryCount == 1 ? "%d item" : "%d items", entryCount);
    Theme_StatusBar(ImVec2(x, y), ImVec2(x + width, y + STATUS_HEIGHT), status);
}

void FileBrowser::DrawContent()
{
    if(!loaded)
    {
        loaded = true;
        Navigate(path);
    }

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 size = ImGui::GetContentRegionAvail();
    float listY = pos.y + TOOLBAR_HEIGHT + HEADER_HEIGHT;
    float listHeight = size.y - TOOLBAR_HEIGHT - HEADER_HEIGHT - STATUS_HEIGHT;
    bool scrolls = entryCount * ROW_HEIGHT > listHeight;

    DrawToolbar(pos.x, pos.y, size.x);
    DrawHeader(pos.x, pos.y + TOOLBAR_HEIGHT, scrolls ? size.x - ImGui::GetStyle().ScrollbarSize : size.x);
    DrawList(pos.x, listY, size.x, listHeight);
    DrawStatus(pos.x, listY + listHeight, size.x);
}
