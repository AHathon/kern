#include "libraries/hardware/mmio_vars.h"
#include "libraries/hardware/debug.h"
#include "libraries/services/svc.h"
#include "libraries/ipc/server.h"
#include "libraries/ipc/fs.h"
#include "libraries/libc/string.h"
#include "kernel/memory/kMemoryMap.h"
#include "libraries/types.h"
#include "fatfs/ff.h"

#define FS_BUF_VA       0x40000000UL
#define FS_MAX_LIST     (IPC_MAX_COPY / sizeof(FsDirEntry))
#define FS_LABEL_SIZE   34

//Drive prefix plus a client path
#define FS_TARGET_SIZE  (FS_MAX_PATH + 2)

typedef struct
{
    FATFS fat;
    bool mounted;
    char label[FS_LABEL_SIZE];
} FsVolume;

static_assert(sizeof(FsPathIn) <= sizeof(IpcMessage::data), "FsPathIn doesn't fit in an IPC message");

PARTITION VolToPart[FF_VOLUMES] = { {0, 1}, {0, 2} };

static FsVolume volumes[FF_VOLUMES];
static FsDirEntry *const listBuf = (FsDirEntry*)FS_BUF_VA;

static FIL file;
static char filePath[FS_TARGET_SIZE];
static bool fileOpen = false;
static DIR dir;
static FILINFO info;

static char FS_ToLower(char c)
{
    return (c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c;
}

static bool FS_Resolve(const char *path, char *target)
{
    while(*path == '/')
        path++;

    for(int v = 0; v < FF_VOLUMES; v++)
    {
        if(!volumes[v].mounted)
            continue;

        const char *label = volumes[v].label;
        const char *rest = path;
        for(; *label && FS_ToLower(*rest) == FS_ToLower(*label); label++)
            rest++;
        if(*label || (*rest && *rest != '/'))
            continue;

        target[0] = '0' + v;
        target[1] = ':';
        strcpy(&target[2], rest);
        return true;
    }
    return false;
}

//FatFs reports the wrong object type the same way as a missing path
static long FS_OpenError(FRESULT r, const char *target, long wrongType)
{
    if(r != FR_NO_FILE && r != FR_NO_PATH && r != FR_INVALID_NAME)
        return FS_ERR_IO;
    return f_stat(target, &info) == FR_OK ? wrongType : FS_ERR_NOT_FOUND;
}

static void FS_FillEntry(FsDirEntry *out, const char *name, uint8_t attr, uint64_t size)
{
    size_t n = 0;
    for(; name[n] && n < sizeof(out->name) - 1; n++)
        out->name[n] = name[n];
    out->name[n] = 0;
    out->attr = attr;
    out->size = size;
}

static void FS_CloseFile()
{
    if(fileOpen)
        f_close(&file);
    fileOpen = false;
}

static long FS_ReadFile(ServerRequest *req, const char *path, uint64_t offset)
{
    char target[FS_TARGET_SIZE];
    if(!FS_Resolve(path, target))
        return FS_ERR_NOT_FOUND;

    //Clients read in chunks; reopening for each one would rewalk the cluster chain
    if(!fileOpen || strcmp(filePath, target) != 0)
    {
        FS_CloseFile();
        FRESULT r = f_open(&file, target, FA_READ);
        if(r != FR_OK)
            return FS_OpenError(r, target, FS_ERR_IS_DIR);
        strcpy(filePath, target);
        fileOpen = true;
    }

    uint32_t size = req->bufCapacity[0];
    if(size > IPC_MAX_COPY)
        size = IPC_MAX_COPY;

    UINT n = 0;
    if(f_lseek(&file, offset) != FR_OK || f_read(&file, (void*)FS_BUF_VA, size, &n) != FR_OK)
    {
        //FatFs latches the error in the FIL
        FS_CloseFile();
        return FS_ERR_IO;
    }

    FsReadFileOut *out = (FsReadFileOut*)req->out;
    out->bytesRead = n;
    out->fileSize = f_size(&file);
    req->outSize = sizeof(*out);
    req->replyBufs[0].ptr = (void*)FS_BUF_VA;
    req->replyBufs[0].size = n;
    return FS_OK;
}

static bool FS_IsRoot(const char *path)
{
    while(*path == '/')
        path++;
    return !*path;
}

static uint32_t FS_ListCapacity(ServerRequest *req)
{
    uint32_t cap = req->bufCapacity[0] / sizeof(FsDirEntry);
    return cap < FS_MAX_LIST ? cap : FS_MAX_LIST;
}

static long FS_ListReply(ServerRequest *req, uint32_t count)
{
    FsListDirOut *out = (FsListDirOut*)req->out;
    out->count = count;
    req->outSize = sizeof(*out);
    req->replyBufs[0].ptr = listBuf;
    req->replyBufs[0].size = count * sizeof(FsDirEntry);
    return FS_OK;
}

static long FS_ListVolumes(ServerRequest *req, uint32_t start)
{
    uint32_t max = FS_ListCapacity(req);
    uint32_t count = 0;
    uint32_t index = 0;
    for(int v = 0; v < FF_VOLUMES && count < max; v++)
    {
        if(!volumes[v].mounted)
            continue;
        if(index++ >= start)
            FS_FillEntry(&listBuf[count++], volumes[v].label, FS_ATTR_DIRECTORY, 0);
    }
    return FS_ListReply(req, count);
}

static long FS_ListDir(ServerRequest *req, const char *path, uint32_t start)
{
    if(FS_IsRoot(path))
        return FS_ListVolumes(req, start);

    char target[FS_TARGET_SIZE];
    if(!FS_Resolve(path, target))
        return FS_ERR_NOT_FOUND;

    FRESULT r = f_opendir(&dir, target);
    if(r != FR_OK)
        return FS_OpenError(r, target, FS_ERR_NOT_DIR);

    uint32_t max = FS_ListCapacity(req);
    uint32_t count = 0;
    for(uint32_t index = 0; count < max; index++)
    {
        r = f_readdir(&dir, &info);
        if(r != FR_OK || !info.fname[0])
            break;
        if(index >= start)
            FS_FillEntry(&listBuf[count++], info.fname, info.fattrib, info.fsize);
    }
    f_closedir(&dir);

    return r == FR_OK ? FS_ListReply(req, count) : FS_ERR_IO;
}

static long FS_Handle(ServerRequest *req, void *ctx)
{
    UNUSED(ctx);
    if(req->inSize < sizeof(FsPathIn) || req->bufCount < 1)
        return FS_ERR_INVALID;

    const FsPathIn *in = (const FsPathIn*)req->in;
    char path[FS_MAX_PATH];
    size_t n = 0;
    for(; in->path[n] && n < sizeof(path) - 1; n++)
        path[n] = in->path[n];
    path[n] = 0;

    switch(req->cmd)
    {
        case FS_CMD_READ_FILE:
            return FS_ReadFile(req, path, in->offset);
        case FS_CMD_LIST_DIR:
            return FS_ListDir(req, path, in->offset);
        default:
            return FS_ERR_INVALID;
    }
}

static bool FS_Mount(int v)
{
    FsVolume *vol = &volumes[v];
    char drive[] = { (char)('0' + v), ':', 0 };

    FRESULT r = f_mount(&vol->fat, drive, 1);
    if(r != FR_OK)
    {
        LOG("[fs] nothing mounted on partition %d (err %d)\n", VolToPart[v].pt, r);
        return false;
    }

    if(f_getlabel(drive, vol->label, 0) != FR_OK || !vol->label[0])
    {
        strcpy(vol->label, "VOLUME0");
        vol->label[6] += v;
    }

    LOG("[fs] %s %s on partition %d\n", vol->label, vol->fat.fs_type == FS_EXFAT ? "exFAT" : "FAT", VolToPart[v].pt);
    vol->mounted = true;
    return true;
}

int main()
{
    //KIPs run at EL1 but link their own copy of the libraries; point them at the kernel's linear map
    MMIO_ADDR = MMIO_BASE + KERNEL_VIRT_BASE;

    long port = serverRegister(FS_PORT_NAME);
    if(port < 0)
    {
        ERROR("[fs] couldn't register port (err -%d)\n", -port);
        return 0;
    }

    if(svcMapMem((void*)FS_BUF_VA, IPC_MAX_COPY) != SVC_OK)
    {
        ERROR("[fs] couldn't map transfer buffer\n");
        return 0;
    }

    //Keep serving even without a card so clients get errors instead of hanging
    bool mounted = false;
    for(int v = 0; v < FF_VOLUMES; v++)
        mounted |= FS_Mount(v);
    if(!mounted)
        ERROR("[fs] no volumes; all requests will fail\n");

    long r = serverRun(port, FS_Handle, 0);
    ERROR("[fs] server stopped (err -%d)\n", -r);
    return 0;
}
