#include "services.h"
#include "libraries/services/svc.h"
#include "desktop.h"

Service fs;
Service gfx;
Service hid;

void _init()
{
    if(gfxInitialize(&gfx) != GFX_OK)
    {
        svcDebug("couldn't connect to gfx");
        svcExitProcess();
    }

    if(hidInitialize(&hid) != HID_OK)
        svcDebug("couldn't connect to hid");

    if(fsInitialize(&fs) != FS_OK)
        svcDebug("couldn't connect to fs");
}

void _fini()
{
    fsExit(&fs);
    hidExit(&hid);
    gfxExit(&gfx);
}

int main()
{
    Desktop desktop;
    desktop.Initialize();

    while(desktop.IsRunning())
        desktop.Update();

    desktop.Finalize();
    return 0;
}
