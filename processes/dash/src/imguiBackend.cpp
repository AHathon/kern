#include "imguiBackend.h"
#include "libraries/services/svc.h"

static_assert(sizeof(ImDrawVert) == sizeof(GfxVertex) && offsetof(ImDrawVert, col) == offsetof(GfxVertex, color), "ImDrawVert must match GfxVertex");
static_assert(sizeof(ImDrawIdx) == sizeof(uint16_t), "gfx takes 16-bit indices");

#define CLEAR_COLOR 0x00202830

static Service *gfx = nullptr;

void ImGuiBackend_Init(Service *service)
{
    gfx = service;

    ImGuiIO &io = ImGui::GetIO();
    io.BackendRendererName = "gfx";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
    ImGui::GetPlatformIO().Renderer_TextureMaxWidth = 2048;
    ImGui::GetPlatformIO().Renderer_TextureMaxHeight = 2048;
}

static void CreateTexture(ImTextureData *tex)
{
    IM_ASSERT(tex->Format == ImTextureFormat_RGBA32);

    uint32_t id = 0;
    if(gfxCreateTexture(gfx, tex->Width, tex->Height, &id) != GFX_OK ||
        gfxUpdateTexture(gfx, id, 0, 0, tex->Width, tex->Height, tex->GetPixels(), tex->GetPitch()) != GFX_OK)
        svcDebug("couldn't create texture");

    tex->SetTexID((ImTextureID)id);
    tex->SetStatus(ImTextureStatus_OK);
}

static void UpdateTextures(ImDrawData *data)
{
    if(!data->Textures)
        return;

    for(ImTextureData *tex : *data->Textures)
    {
        if(tex->Status == ImTextureStatus_WantCreate)
            CreateTexture(tex);
        else if(tex->Status == ImTextureStatus_WantUpdates)
        {
            for(const ImTextureRect &r : tex->Updates)
                gfxUpdateTexture(gfx, (uint32_t)tex->GetTexID(), r.x, r.y, r.w, r.h, tex->GetPixelsAt(r.x, r.y), tex->GetPitch());
            tex->SetStatus(ImTextureStatus_OK);
        }
        else if(tex->Status == ImTextureStatus_WantDestroy)
        {
            gfxDestroyTexture(gfx, (uint32_t)tex->GetTexID());
            tex->SetTexID(ImTextureID_Invalid);
            tex->SetStatus(ImTextureStatus_Destroyed);
        }
    }
}

void ImGuiBackend_Render(ImDrawData *data)
{
    UpdateTextures(data);
    gfxBeginFrame(gfx, CLEAR_COLOR);

    ImVector<GfxDrawCmd> cmds;
    for(const ImDrawList *list : data->CmdLists)
    {
        cmds.resize(0);
        for(const ImDrawCmd &cmd : list->CmdBuffer)
        {
            if(cmd.UserCallback)
                continue;

            GfxDrawCmd out;
            out.clipX0 = (int32_t)cmd.ClipRect.x;
            out.clipY0 = (int32_t)cmd.ClipRect.y;
            out.clipX1 = (int32_t)cmd.ClipRect.z;
            out.clipY1 = (int32_t)cmd.ClipRect.w;
            out.texture = (uint32_t)cmd.GetTexID();
            out.idxOffset = cmd.IdxOffset;
            out.vtxOffset = cmd.VtxOffset;
            out.elemCount = cmd.ElemCount;
            cmds.push_back(out);
        }

        long r = gfxDrawList(gfx, (const GfxVertex*)list->VtxBuffer.Data, list->VtxBuffer.Size,
                             list->IdxBuffer.Data, list->IdxBuffer.Size, cmds.Data, cmds.Size);
        if(r != GFX_OK)
            svcDebug("draw list rejected");
    }

    gfxEndFrame(gfx);
}
