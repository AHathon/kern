#pragma once

#include "libraries/types.h"
#include "libraries/ipc/service.h"

#define FS_OK               0
#define FS_ERR_NOT_FOUND    -100
#define FS_ERR_IO           -101
#define FS_ERR_INVALID      -102
#define FS_ERR_NOT_DIR      -103
#define FS_ERR_IS_DIR       -104

#define FS_MAX_PATH         128
#define FS_ATTR_DIRECTORY   0x10

typedef struct
{
    char name[13];
    uint8_t attr;
    uint16_t _pad;
    uint32_t size;
} FsDirEntry;

#ifdef __cplusplus
extern "C" {
#endif

long fsInitialize(Service *fs);
void fsExit(Service *fs);
long fsReadFile(Service *fs, const char *path, uint32_t offset, void *buf, uint32_t size,
                uint32_t *bytesRead, uint32_t *fileSize);
long fsListDir(Service *fs, const char *path, uint32_t start, FsDirEntry *entries, uint32_t maxEntries,
               uint32_t *count);

#ifdef __cplusplus
}
#endif
