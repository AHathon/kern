#pragma once

#include "libraries/types.h"
#include "libraries/ipc/service.h"

#define FS_PORT_NAME        "fs"

#define FS_OK               0
#define FS_ERR_NOT_FOUND    -100
#define FS_ERR_IO           -101
#define FS_ERR_INVALID      -102
#define FS_ERR_NOT_DIR      -103
#define FS_ERR_IS_DIR       -104

#define FS_MAX_PATH         200
#define FS_MAX_NAME         256
#define FS_ATTR_DIRECTORY   0x10

typedef struct
{
    char name[FS_MAX_NAME];
    uint8_t attr;
    uint8_t _pad[7];
    uint64_t size;
} FsDirEntry;

typedef enum
{
    FS_CMD_READ_FILE = 1,
    FS_CMD_LIST_DIR  = 2
} FsCommand;

typedef struct
{
    uint64_t offset;
    char path[FS_MAX_PATH];
} FsPathIn;

typedef struct
{
    uint32_t bytesRead;
    uint32_t _pad;
    uint64_t fileSize;
} FsReadFileOut;

typedef struct
{
    uint32_t count;
} FsListDirOut;

#ifdef __cplusplus
extern "C" {
#endif

long fsInitialize(Service *fs);
void fsExit(Service *fs);
long fsReadFile(Service *fs, const char *path, uint64_t offset, void *buf, uint32_t size,
                uint32_t *bytesRead, uint64_t *fileSize);
long fsListDir(Service *fs, const char *path, uint32_t start, FsDirEntry *entries, uint32_t maxEntries,
               uint32_t *count);

#ifdef __cplusplus
}
#endif
