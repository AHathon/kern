#include "libraries/hardware/emmc.h"
#include "libraries/hardware/mmio_vars.h"
#include "libraries/hardware/debug.h"
#include "libraries/services/svc.h"
#include "libraries/services/ipc.h"
#include "fs.h"
#include "kernel/memory/kMemoryMap.h"
#include "libraries/types.h"
#include "fat32.h"

//Reply data is staged here; the kernel copies it into the client's buffer
#define FS_BUF_VA       0x40000000UL
#define FS_MAX_LIST     64

static Fat32_t fat;
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
        return 0; //. and ..
    if(list->index++ < list->start)
        return 0;

    FsDirEntry *out = &listBuf[list->count++];
    for(int i = 0; i < (int)sizeof(out->name); i++)
        out->name[i] = entry->name[i];
    out->attr = entry->attr;
    out->size = entry->size;
    return list->count >= list->max;
}

//Both handlers read the request before writing the reply over it
static int32_t FS_HandleReadFile(IpcMessage *msg, FsRequest *req)
{
    Fat32Entry_t entry;
    if(Fat32_Open(&fat, req->path, &entry) != 0)
        return FS_ERR_NOT_FOUND;
    if(entry.attr & FAT_ATTR_DIRECTORY)
        return FS_ERR_IS_DIR;

    uint32_t size = msg->bufCount ? msg->bufs[0].size : 0;
    if(size > IPC_MAX_COPY)
        size = IPC_MAX_COPY;

    long n = Fat32_ReadAt(&fat, &entry, req->offset, (uint8_t*)FS_BUF_VA, size);
    if(n < 0)
        return FS_ERR_IO;

    ((FsReply*)msg->data)->total = entry.size;
    msg->dataSize = sizeof(FsReply);
    msg->bufCount = 1;
    msg->bufs[0].va = FS_BUF_VA;
    msg->bufs[0].size = n;
    return FS_OK;
}

static int32_t FS_HandleListDir(IpcMessage *msg, FsRequest *req)
{
    Fat32Entry_t dir;
    if(Fat32_Open(&fat, req->path, &dir) != 0)
        return FS_ERR_NOT_FOUND;
    if(!(dir.attr & FAT_ATTR_DIRECTORY))
        return FS_ERR_NOT_DIR;

    FsListCtx list = { req->offset, 0, 0, FS_MAX_LIST };
    uint32_t cap = msg->bufCount ? msg->bufs[0].size / sizeof(FsDirEntry) : 0;
    if(cap < list.max)
        list.max = cap;

    if(list.max && Fat32_ListDir(&fat, dir.cluster, FS_ListVisitor, &list) != 0)
        return FS_ERR_IO;

    ((FsReply*)msg->data)->total = list.count;
    msg->dataSize = sizeof(FsReply);
    msg->bufCount = 1;
    msg->bufs[0].va = (uintptr_t)listBuf;
    msg->bufs[0].size = list.count * sizeof(FsDirEntry);
    return FS_OK;
}

int main()
{
    //KIPs run at EL1 but link their own copy of the libraries; point them at the kernel's linear map
    MMIO_ADDR = MMIO_BASE + KERNEL_VIRT_BASE;

    long port = svcCreatePort(FS_PORT_NAME);
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
    int mounted = EMMC_Init() == 0 && Fat32_Mount(&fat) == 0;
    if(!mounted)
        ERROR("[fs] no boot volume; all requests will fail\n");

    long reply = 0;
    while(1)
    {
        long session = svcReplyAndReceive(port, reply);
        if(session < 0)
        {
            //A failed reply still leaves us able to receive; anything else is fatal
            if(!reply)
            {
                ERROR("[fs] receive failed (err -%d)\n", -session);
                return 0;
            }
            reply = 0;
            continue;
        }

        IpcMessage *msg = Ipc_GetMessage();
        FsRequest *req = (FsRequest*)msg->data;
        req->path[FS_MAX_PATH - 1] = 0;

        int32_t result;
        if(!mounted)
            result = FS_ERR_IO;
        else if(msg->dataSize < sizeof(FsRequest))
            result = FS_ERR_INVALID;
        else if(msg->cmd == FS_CMD_READ_FILE)
            result = FS_HandleReadFile(msg, req);
        else if(msg->cmd == FS_CMD_LIST_DIR)
            result = FS_HandleListDir(msg, req);
        else
            result = FS_ERR_INVALID;

        if(result != FS_OK)
        {
            msg->dataSize = 0;
            msg->bufCount = 0;
        }
        msg->cmd = (uint32_t)result;
        reply = session;
    }
}
