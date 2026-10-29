#include "libraries/hardware/emmc.h"
#include "libraries/hardware/mmio_vars.h"
#include "libraries/hardware/debug.h"
#include "libraries/services/svc.h"
#include "libraries/ipc/server.h"
#include "fs.h"
#include "kernel/memory/kMemoryMap.h"
#include "libraries/types.h"
#include "fat32.h"

#define FS_BUF_VA       0x40000000UL
#define FS_MAX_LIST     64

static Fat32_t fat;
static int mounted = 0;
static FsDirEntry listBuf[FS_MAX_LIST];

typedef struct
{
    uint32_t start;
    uint32_t index;
    uint32_t count;
    uint32_t max;
} FsListCtx;

static int FS_ListVisitor(Fat32_t *fs, Fat32Entry_t *entry, void *ctx)
{
    UNUSED(fs);
    FsListCtx *list = (FsListCtx*)ctx;
    if(entry->name[0] == '.')
        return 0;
    if(list->index++ < list->start)
        return 0;

    FsDirEntry *out = &listBuf[list->count++];
    size_t n = 0;
    for(; entry->name[n] && n < sizeof(out->name) - 1; n++)
        out->name[n] = entry->name[n];
    out->name[n] = 0;
    out->attr = entry->attr;
    out->size = entry->size;
    return list->count >= list->max;
}

static long FS_ReadFile(ServerRequest *req, const char *path, uint32_t offset)
{
    Fat32Entry_t entry;
    if(Fat32_Open(&fat, path, &entry) != 0)
        return FS_ERR_NOT_FOUND;
    if(entry.attr & FAT_ATTR_DIRECTORY)
        return FS_ERR_IS_DIR;

    uint32_t size = req->bufCapacity[0];
    if(size > IPC_MAX_COPY)
        size = IPC_MAX_COPY;

    long n = Fat32_ReadAt(&fat, &entry, offset, (uint8_t*)FS_BUF_VA, size);
    if(n < 0)
        return FS_ERR_IO;

    FsReadFileOut *out = (FsReadFileOut*)req->out;
    out->bytesRead = n;
    out->fileSize = entry.size;
    req->outSize = sizeof(*out);
    req->replyBufs[0].ptr = (void*)FS_BUF_VA;
    req->replyBufs[0].size = n;
    return FS_OK;
}

static long FS_ListDir(ServerRequest *req, const char *path, uint32_t start)
{
    Fat32Entry_t dir;
    if(Fat32_Open(&fat, path, &dir) != 0)
        return FS_ERR_NOT_FOUND;
    if(!(dir.attr & FAT_ATTR_DIRECTORY))
        return FS_ERR_NOT_DIR;

    FsListCtx list = { start, 0, 0, FS_MAX_LIST };
    uint32_t cap = req->bufCapacity[0] / sizeof(FsDirEntry);
    if(cap < list.max)
        list.max = cap;

    if(list.max && Fat32_ListDir(&fat, dir.cluster, FS_ListVisitor, &list) != 0)
        return FS_ERR_IO;

    FsListDirOut *out = (FsListDirOut*)req->out;
    out->count = list.count;
    req->outSize = sizeof(*out);
    req->replyBufs[0].ptr = listBuf;
    req->replyBufs[0].size = list.count * sizeof(FsDirEntry);
    return FS_OK;
}

static long FS_Handle(ServerRequest *req, void *ctx)
{
    UNUSED(ctx);
    if(!mounted)
        return FS_ERR_IO;
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
    mounted = EMMC_Init() == 0 && Fat32_Mount(&fat) == 0;
    if(!mounted)
        ERROR("[fs] no boot volume; all requests will fail\n");

    long r = serverRun(port, FS_Handle, 0);
    ERROR("[fs] server stopped (err -%d)\n", -r);
    return 0;
}
