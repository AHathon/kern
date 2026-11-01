#pragma once

class Window
{
public:
    Window(const char *title, float x, float y, float width, float height, bool open = true, float padding = 10.0f);
    ~Window() = default;

    void Draw();

    void Open();
    void Close();
    void Minimize();
    void ToggleMaximize();
    void ToggleFromTaskbar();

    const char *Title() const { return title; }
    bool IsOpen() const { return open; }
    bool IsActive() const { return active == this; }

    static void ClearActive() { active = nullptr; }

protected:
    virtual void DrawContent() {}

private:
    bool Begin();
    void End();
    void DrawCaptionButtons(float right, float top);
    void HandleCaption(float width);
    void MoveTo(float newX, float newY);

    static Window *active;

    const char *title;
    float x, y, width, height;
    float padding;
    float restoreX = 0, restoreY = 0, restoreWidth = 0, restoreHeight = 0;
    float grabX = 0, grabY = 0;
    bool open;
    bool minimized = false;
    bool maximized = false;
    bool wantPlace = true;
    bool wantFocus = false;
};
