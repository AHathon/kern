#include "imgui/imgui.h"
#include "libraries/services/svc.h"
#include "libc/stdio.h"

static void UpdateTextures(ImDrawData *data)
{
    for(ImTextureData *tex : *data->Textures)
    {
        if(tex->Status == ImTextureStatus_OK)
            continue;

        char line[96];
        snprintf(line, sizeof(line), "texture %d: status %d, %dx%d", tex->UniqueID, (int)tex->Status, tex->Width, tex->Height);
        svcDebug(line);

        if(tex->Status == ImTextureStatus_WantDestroy)
        {
            tex->SetTexID(ImTextureID_Invalid);
            tex->SetStatus(ImTextureStatus_Destroyed);
        }
        else
        {
            tex->SetTexID((ImTextureID)(uintptr_t)tex->UniqueID + 1);
            tex->SetStatus(ImTextureStatus_OK);
        }
    }
}

static void RenderDrawData(ImDrawData *data)
{
    UpdateTextures(data);

    int commands = 0;
    for(ImDrawList *list : data->CmdLists)
        commands += list->CmdBuffer.Size;

    char line[96];
    snprintf(line, sizeof(line), "frame: %d lists, %d cmds, %d vtx, %d idx", data->CmdLists.Size, commands, data->TotalVtxCount, data->TotalIdxCount);
    svcDebug(line);
}

int main()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.DisplaySize = ImVec2(800, 480);
    io.DeltaTime = 1.0f / 60.0f;
    io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
    ImGui::GetPlatformIO().Renderer_TextureMaxWidth = 2048;
    ImGui::GetPlatformIO().Renderer_TextureMaxHeight = 2048;

    float value = 0.25f;
    for(int frame = 0; frame < 3; frame++)
    {
        ImGui::NewFrame();
        ImGui::Begin("dash");
        ImGui::Text("frame %d, pi %.3f", frame, 3.14159f);
        ImGui::SliderFloat("value", &value, 0.0f, 1.0f);
        ImGui::Button("ok");
        ImGui::End();
        ImGui::Render();
        RenderDrawData(ImGui::GetDrawData());
    }

    ImGui::DestroyContext();
    svcDebug("done");
    return 0;
}
