#pragma once

#include "libraries/types.h"
#include "libraries/ipc/fs.h"

#define FS_PORT_NAME        "fs"

typedef enum
{
    FS_CMD_READ_FILE = 1,
    FS_CMD_LIST_DIR  = 2
} FsCommand;

typedef struct
{
    uint32_t offset;
    char path[FS_MAX_PATH];
} FsPathIn;

typedef struct
{
    uint32_t bytesRead;
    uint32_t fileSize;
} FsReadFileOut;

typedef struct
{
    uint32_t count;
} FsListDirOut;
