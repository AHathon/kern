#include "theme.h"
#include "wallpaper.h"
#include "backend/imguiBackend.h"

void Wallpaper::Finalize()
{
    ClearImage();
}

void Wallpaper::SetColor(uint32_t rgb)
{
    ImGuiBackend_SetClearColor(rgb);
}

bool Wallpaper::SetImage(const uint32_t *pixels, uint32_t width, uint32_t height)
{
    return image.Load(pixels, width, height);
}

bool Wallpaper::SetBmp(const void *data, size_t size)
{
    return image.LoadBmp(data, size);
}

void Wallpaper::ClearImage()
{
    image.Unload();
}

void Wallpaper::Draw()
{
    if(image.Texture())
        ImGui::GetBackgroundDrawList()->AddImage(ImTextureRef((ImTextureID)image.Texture()), ImVec2(0, 0), ImGui::GetIO().DisplaySize);
}
