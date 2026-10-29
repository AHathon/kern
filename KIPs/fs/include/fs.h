#pragma once

#include "libraries/types.h"
#include "libraries/services/ipc.h"
#include "libraries/services/svc.h"

#define FS_PORT_NAME        "fs"
#define FS_MAX_PATH         128

#define FS_OK               0
#define FS_ERR_NOT_FOUND    -100
#define FS_ERR_IO           -101
#define FS_ERR_INVALID      -102
#define FS_ERR_NOT_DIR      -103
#define FS_ERR_IS_DIR       -104

#define FS_ATTR_DIRECTORY   0x10

typedef enum
{
    FS_CMD_READ_FILE = 1,
    FS_CMD_LIST_DIR  = 2
} FsCommand;

typedef struct
{
    char name[13];
    uint8_t attr;
    uint16_t _pad;
    uint32_t size;
} FsDirEntry;

//Request data
typedef struct
{
    uint32_t offset;
    char path[FS_MAX_PATH];
} FsRequest;

typedef struct
{
    uint32_t total;
} FsReply;

static inline long FS_Call(long session, FsCommand cmd, const char *path, uint32_t offset, void *buf, uint32_t size)
{
    IpcMessage *msg = Ipc_GetMessage();
    FsRequest *req = (FsRequest*)msg->data;

    size_t n = 0;
    for(; path[n]; n++)
    {
        if(n >= FS_MAX_PATH - 1)
            return FS_ERR_INVALID;
        req->path[n] = path[n];
    }
    req->path[n] = 0;
    req->offset = offset;

    msg->cmd = (uint32_t)cmd;
    msg->dataSize = sizeof(FsRequest);
    msg->bufCount = 1;
    msg->bufs[0].va = (uintptr_t)buf;
    msg->bufs[0].size = size;

    long r = svcSendSyncRequest(session);
    if(r != SVC_OK)
        return r;
    return (int32_t)msg->cmd;
}

//Returns bytes read (0 at EOF) or a negative FS_ERR_* / SVC_ERR_*
static inline long FS_ReadFile(long session, const char *path, uint32_t offset, void *buf, uint32_t size, uint32_t *fileSize)
{
    long r = FS_Call(session, FS_CMD_READ_FILE, path, offset, buf, size);
    if(r != FS_OK)
        return r;

    IpcMessage *msg = Ipc_GetMessage();
    if(fileSize)
        *fileSize = ((FsReply*)msg->data)->total;
    return msg->bufs[0].size;
}

static inline long FS_ListDir(long session, const char *path, uint32_t start, FsDirEntry *entries, uint32_t maxEntries)
{
    long r = FS_Call(session, FS_CMD_LIST_DIR, path, start, entries, maxEntries * sizeof(FsDirEntry));
    if(r != FS_OK)
        return r;
    return Ipc_GetMessage()->bufs[0].size / sizeof(FsDirEntry);
}
