#pragma once

#include "libraries/types.h"

#define FAT_ATTR_DIRECTORY  0x10
#define FAT_LABEL_SIZE      11

typedef struct
{
    uint32_t fatLba;
    uint32_t dataLba;
    uint32_t sectorsPerCluster;
    uint32_t clusterCount;
    uint32_t rootCluster;
    char label[FAT_LABEL_SIZE + 1];
} Fat32_t;

typedef struct
{
    char name[255];
    uint8_t attr;
    uint32_t cluster;
    uint32_t size;
} Fat32Entry_t;

typedef int (*Fat32_Visitor)(Fat32_t *fs, Fat32Entry_t *entry, void *ctx);

int Fat32_Mount(Fat32_t *fs);
const char *Fat32_StripLabel(Fat32_t *fs, const char *path);
int Fat32_ListDir(Fat32_t *fs, uint32_t dirCluster, Fat32_Visitor visit, void *ctx);
int Fat32_Find(Fat32_t *fs, uint32_t dirCluster, const char *name, Fat32Entry_t *out);
int Fat32_Open(Fat32_t *fs, const char *path, Fat32Entry_t *out);
long Fat32_ReadAt(Fat32_t *fs, Fat32Entry_t *entry, uint32_t offset, uint8_t *buf, uint32_t size);
