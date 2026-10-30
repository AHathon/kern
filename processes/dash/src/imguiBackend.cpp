#include "imguiBackend.h"
#include "libraries/services/svc.h"

static_assert(sizeof(ImDrawVert) == sizeof(GfxVertex) && offsetof(ImDrawVert, col) == offsetof(GfxVertex, color), "ImDrawVert must match GfxVertex");
static_assert(sizeof(ImDrawIdx) == sizeof(uint16_t), "gfx takes 16-bit indices");

#define CLEAR_COLOR 0x00202830

#define USAGE_A            0x04
#define USAGE_Z             0x1D
#define USAGE_1             0x1E
#define USAGE_9             0x26
#define USAGE_0             0x27
#define USAGE_SLASH         0x38
#define USAGE_CAPS_LOCK     0x39
#define USAGE_F1            0x3A
#define USAGE_F12           0x45
#define USAGE_COUNT         0x100

static Service *gfx = nullptr;
static HidState previous;
static bool capsLock = false;

void ImGuiBackend_Init(Service *service)
{
    gfx = service;

    ImGuiIO &io = ImGui::GetIO();
    io.BackendRendererName = "gfx";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
    io.MouseDrawCursor = true; //there's no hardware cursor
    ImGui::GetPlatformIO().Renderer_TextureMaxWidth = 2048;
    ImGui::GetPlatformIO().Renderer_TextureMaxHeight = 2048;
}

static ImGuiKey UsageToKey(int usage)
{
    if(usage >= USAGE_A && usage <= USAGE_Z)
        return (ImGuiKey)(ImGuiKey_A + usage - USAGE_A);
    if(usage >= USAGE_1 && usage <= USAGE_9)
        return (ImGuiKey)(ImGuiKey_1 + usage - USAGE_1);
    if(usage >= USAGE_F1 && usage <= USAGE_F12)
        return (ImGuiKey)(ImGuiKey_F1 + usage - USAGE_F1);

    switch(usage)
    {
        case USAGE_0: return ImGuiKey_0;
        case 0x28: return ImGuiKey_Enter;
        case 0x29: return ImGuiKey_Escape;
        case 0x2A: return ImGuiKey_Backspace;
        case 0x2B: return ImGuiKey_Tab;
        case 0x2C: return ImGuiKey_Space;
        case 0x2D: return ImGuiKey_Minus;
        case 0x2E: return ImGuiKey_Equal;
        case 0x2F: return ImGuiKey_LeftBracket;
        case 0x30: return ImGuiKey_RightBracket;
        case 0x31: return ImGuiKey_Backslash;
        case 0x33: return ImGuiKey_Semicolon;
        case 0x34: return ImGuiKey_Apostrophe;
        case 0x35: return ImGuiKey_GraveAccent;
        case 0x36: return ImGuiKey_Comma;
        case 0x37: return ImGuiKey_Period;
        case USAGE_SLASH: return ImGuiKey_Slash;
        case USAGE_CAPS_LOCK: return ImGuiKey_CapsLock;
        case 0x49: return ImGuiKey_Insert;
        case 0x4A: return ImGuiKey_Home;
        case 0x4B: return ImGuiKey_PageUp;
        case 0x4C: return ImGuiKey_Delete;
        case 0x4D: return ImGuiKey_End;
        case 0x4E: return ImGuiKey_PageDown;
        case 0x4F: return ImGuiKey_RightArrow;
        case 0x50: return ImGuiKey_LeftArrow;
        case 0x51: return ImGuiKey_DownArrow;
        case 0x52: return ImGuiKey_UpArrow;
        default: return ImGuiKey_None;
    }
}

static char UsageToChar(int usage, bool shift)
{
    //US layout, indexed from USAGE_1; holes are keys that don't type anything
    static const char plain[]   = "1234567890\0\0\0\0 -=[]\\\0;'`,./";
    static const char shifted[] = "!@#$%^&*()\0\0\0\0 _+{}|\0:\"~<>?";

    if(usage >= USAGE_A && usage <= USAGE_Z)
        return (shift != capsLock ? 'A' : 'a') + usage - USAGE_A;
    if(usage >= USAGE_1 && usage <= USAGE_SLASH)
        return (shift ? shifted : plain)[usage - USAGE_1];
    return 0;
}

void ImGuiBackend_UpdateInput(const HidState *state)
{
    ImGuiIO &io = ImGui::GetIO();
    uint8_t mods = state->modifiers;

    io.AddKeyEvent(ImGuiMod_Ctrl, mods & HID_MOD_CTRL);
    io.AddKeyEvent(ImGuiMod_Shift, mods & HID_MOD_SHIFT);
    io.AddKeyEvent(ImGuiMod_Alt, mods & HID_MOD_ALT);
    io.AddKeyEvent(ImGuiMod_Super, mods & HID_MOD_SUPER);

    for(int usage = 0; usage < USAGE_COUNT; usage++)
    {
        bool down = hidKeyDown(state, usage);
        if(down == (bool)hidKeyDown(&previous, usage))
            continue;

        ImGuiKey key = UsageToKey(usage);
        if(key != ImGuiKey_None)
            io.AddKeyEvent(key, down);
        if(!down)
            continue;

        if(usage == USAGE_CAPS_LOCK)
            capsLock = !capsLock;

        char c = UsageToChar(usage, mods & HID_MOD_SHIFT);
        if(c)
            io.AddInputCharacter(c);
    }

    if(state->mouseX != previous.mouseX || state->mouseY != previous.mouseY)
        io.AddMousePosEvent((float)state->mouseX, (float)state->mouseY);
    if(state->buttons != previous.buttons)
    {
        io.AddMouseButtonEvent(ImGuiMouseButton_Left, state->buttons & HID_BUTTON_LEFT);
        io.AddMouseButtonEvent(ImGuiMouseButton_Right, state->buttons & HID_BUTTON_RIGHT);
        io.AddMouseButtonEvent(ImGuiMouseButton_Middle, state->buttons & HID_BUTTON_MIDDLE);
    }
    if(state->wheel != previous.wheel)
        io.AddMouseWheelEvent(0.0f, (float)(state->wheel - previous.wheel));

    previous = *state;
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
