#include "theme.h"
#include "test_window.h"

void TestWindow::DrawContent()
{
    ImGuiIO &io = ImGui::GetIO();

    progress += io.DeltaTime * 0.25f;
    if(progress > 1.0f)
        progress -= 1.0f;

    ImGui::Text("Dear ImGui %s", IMGUI_VERSION);
    ImGui::Text("%.1f FPS", io.Framerate);
    ImGui::Separator();
    Theme_ProgressBar(progress);
    Theme_Slider("##progress", &progress, 0.0f, 1.0f);
    if(Theme_Button("Button"))
        clicks++;
    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("clicked %d times", clicks);
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::InputTextWithHint("##text", "text", text, sizeof(text));
}
