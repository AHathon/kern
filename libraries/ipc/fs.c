#include "libraries/ipc/fs.h"
#include "fs.h"

static long fs_MakePathIn(FsPathIn *in, const char *path, uint32_t offset)
{
    size_t n = 0;
    for(; path[n]; n++)
    {
        if(n >= FS_MAX_PATH - 1)
            return FS_ERR_INVALID;
        in->path[n] = path[n];
    }
    in->path[n] = 0;
    in->offset = offset;
    return FS_OK;
}

long fsInitialize(Service *fs)
{
    return serviceOpen(fs, FS_PORT_NAME);
}

void fsExit(Service *fs)
{
    serviceClose(fs);
}

long fsReadFile(Service *fs, const char *path, uint32_t offset, void *buf, uint32_t size,
                uint32_t *bytesRead, uint32_t *fileSize)
{
    FsPathIn in;
    FsReadFileOut out;
    if(fs_MakePathIn(&in, path, offset) != FS_OK)
        return FS_ERR_INVALID;

    SfBuffer data = { buf, size };
    long r = serviceDispatch(fs, FS_CMD_READ_FILE, &in, sizeof(in), &out, sizeof(out), &data, 1);
    if(r == FS_OK)
    {
        if(bytesRead) *bytesRead = out.bytesRead;
        if(fileSize) *fileSize = out.fileSize;
    }
    return r;
}

long fsListDir(Service *fs, const char *path, uint32_t start, FsDirEntry *entries, uint32_t maxEntries,
               uint32_t *count)
{
    FsPathIn in;
    FsListDirOut out;
    if(fs_MakePathIn(&in, path, start) != FS_OK)
        return FS_ERR_INVALID;

    SfBuffer list = { entries, maxEntries * sizeof(FsDirEntry) };
    long r = serviceDispatch(fs, FS_CMD_LIST_DIR, &in, sizeof(in), &out, sizeof(out), &list, 1);
    if(r == FS_OK && count)
        *count = out.count;
    return r;
}
