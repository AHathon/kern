#pragma once

#include "libraries/ipc/fs.h"
#include "image.h"
#include "theme.h"
#include "window.h"

#define FILE_BROWSER_MAX_ENTRIES 256
#define FILE_BROWSER_MAX_HISTORY 16

class FileBrowser : public Window
{
public:
    FileBrowser() : Window("File Browser", 130, 50, 540, 340, false, 0.0f) {}
    ~FileBrowser() override;

protected:
    void DrawContent() override;

private:
    bool Load(const char *newPath);
    void Navigate(const char *newPath);
    void Back();
    void Forward();
    void OpenEntry(const FsDirEntry *entry);

    void LoadSprites();
    void DrawIcon(ImVec2 pos, int index);
    bool NavButton(const char *id, ImVec2 center, float direction, bool enabled);
    void DrawToolbar(float x, float y, float width);
    void DrawAddress(float x0, float y0, float x1, float y1);
    void DrawHeader(float x, float y, float width);
    void DrawList(float x, float y, float width, float height);
    void DrawStatus(float x, float y, float width);

    char path[FS_MAX_PATH] = "/";
    FsDirEntry entries[FILE_BROWSER_MAX_ENTRIES];
    int entryCount = 0;
    int selected = -1;
    long error = FS_OK;
    bool truncated = false;
    bool loaded = false;

    char history[FILE_BROWSER_MAX_HISTORY][FS_MAX_PATH];
    int historyCount = 0;
    int historyPos = -1;

    Image icons;
    Image navButtons;
};
