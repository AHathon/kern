#include "libraries/services/svc.h"
#include "libraries/ipc/fs.h"
#include "libraries/hardware/framebuffer.h"
#include "libraries/hardware/mmio_vars.h"
#include "libraries/hardware/debug.h"
#include "libraries/hardware/mmu.h"
#include "kernel/memory/kMemoryMap.h"
#include "libraries/types.h"

//Must match PROC_DIR in the top-level Makefile
#define PROC_DIR    "/bin"
#define PROC_EXT    ".bin"

//Process images are read into here; grown with svcMapMem as needed
#define LOAD_BUF_VA 0x40000000UL

static size_t loadBufSize = 0;
static FsDirEntry entries[16];

static int EnsureLoadBuf(size_t size)
{
    if(size <= loadBufSize)
        return 0;

    size_t grow = (size - loadBufSize + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    if(svcMapMem((void*)(LOAD_BUF_VA + loadBufSize), grow) != SVC_OK)
        return -1;

    loadBufSize += grow;
    return 0;
}

static int HasSuffix(const char *s, const char *suffix)
{
    size_t sLen = 0, sufLen = 0;
    while(s[sLen]) sLen++;
    while(suffix[sufLen]) sufLen++;
    if(sLen < sufLen)
        return 0;

    for(size_t i = 0; i < sufLen; i++)
        if(s[sLen - sufLen + i] != suffix[i])
            return 0;
    return 1;
}

static void LoadProcess(Service *fs, FsDirEntry *entry)
{
    char path[FS_MAX_PATH];
    size_t n = 0;
    for(const char *p = PROC_DIR "/"; *p; p++)
        path[n++] = *p;
    for(const char *p = entry->name; *p; p++)
        path[n++] = *p;
    path[n] = 0;

    if(entry->size == 0 || EnsureLoadBuf(entry->size) != 0)
    {
        ERROR("[init] can't load %s\n", path);
        return;
    }

    //fs hands back at most IPC_MAX_COPY per request
    uint8_t *buf = (uint8_t*)LOAD_BUF_VA;
    for(uint32_t off = 0; off < entry->size; )
    {
        uint32_t chunk = entry->size - off;
        if(chunk > IPC_MAX_COPY)
            chunk = IPC_MAX_COPY;

        uint32_t got = 0;
        long r = fsReadFile(fs, path, off, buf + off, chunk, &got, 0);
        if(r != FS_OK || got == 0)
        {
            ERROR("[init] failed to read %s (err -%d)\n", path, -r);
            return;
        }
        off += got;
    }

    //Process name is the file name minus the extension (kernel keeps 7 chars)
    char name[8];
    for(n = 0; entry->name[n] && entry->name[n] != '.' && n < sizeof(name) - 1; n++)
        name[n] = entry->name[n];
    name[n] = 0;

    long pid = svcCreateProcess(name, buf, entry->size);
    if(pid < 0)
        ERROR("[init] failed to start %s (err -%d)\n", name, -pid);
    else
        LOG("[init] started %s [pid:%d]\n", name, pid);
}

int main()
{
    //KIPs run at EL1 but link their own copy of the libraries; point them at the kernel's linear map
    MMIO_ADDR = MMIO_BASE + KERNEL_VIRT_BASE;
    FB_OFF = KERNEL_VIRT_BASE;

    Framebuffer_Init(MAIN_DISP_WIDTH, MAIN_DISP_HEIGHT);

    Service fs;
    long r = fsInitialize(&fs);
    if(r != FS_OK)
    {
        ERROR("[init] couldn't connect to fs (err -%d)\n", -r);
        return 0;
    }

    uint32_t start = 0;
    while(1)
    {
        uint32_t count = 0;
        r = fsListDir(&fs, PROC_DIR, start, entries, sizeof(entries) / sizeof(entries[0]), &count);
        if(r != FS_OK)
        {
            ERROR("[init] can't list %s (err -%d)\n", PROC_DIR, -r);
            break;
        }

        for(uint32_t i = 0; i < count; i++)
            if(!(entries[i].attr & FS_ATTR_DIRECTORY) && HasSuffix(entries[i].name, PROC_EXT))
                LoadProcess(&fs, &entries[i]);

        if(count < sizeof(entries) / sizeof(entries[0]))
            break;
        start += count;
    }

    fsExit(&fs);
    return 0;
}
