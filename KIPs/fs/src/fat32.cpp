#include "fat32.h"
#include "libraries/hardware/emmc.h"
#include "libraries/hardware/debug.h"

#define FAT32_MASK      0x0FFFFFFF
#define FAT32_BAD       0x0FFFFFF7
#define FAT32_EOC       0x0FFFFFF8

#define DIRENT_SIZE     32
#define ATTR_VOLUME_ID  0x08
#define ATTR_LFN        0x0F

static uint8_t dirSector[EMMC_BLOCK_SIZE];
static uint8_t fatSector[EMMC_BLOCK_SIZE];
static uint8_t tmpSector[EMMC_BLOCK_SIZE];
static uint32_t fatSectorLba = 0xFFFFFFFF;

static uint16_t rd16(const uint8_t *p)
{
    return p[0] | (p[1] << 8);
}

static uint32_t rd32(const uint8_t *p)
{
    return p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24);
}

static char Fat32_ToLower(char c)
{
    return (c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c;
}

static int Fat32_StrEq(const char *a, const char *b)
{
    while(*a && Fat32_ToLower(*a) == Fat32_ToLower(*b))
    {
        a++;
        b++;
    }
    return Fat32_ToLower(*a) == Fat32_ToLower(*b);
}

static int Fat32_IsValidCluster(Fat32_t *fs, uint32_t cluster)
{
    return cluster >= 2 && cluster < FAT32_BAD && cluster < fs->clusterCount + 2;
}

static uint32_t Fat32_ClusterLba(Fat32_t *fs, uint32_t cluster)
{
    return fs->dataLba + (cluster - 2) * fs->sectorsPerCluster;
}

static uint32_t Fat32_NextCluster(Fat32_t *fs, uint32_t cluster)
{
    uint32_t lba = fs->fatLba + (cluster * 4) / EMMC_BLOCK_SIZE;
    if(lba != fatSectorLba)
    {
        if(EMMC_ReadBlocks(lba, 1, fatSector) != 0)
        {
            fatSectorLba = 0xFFFFFFFF;
            return 0;
        }
        fatSectorLba = lba;
    }
    return rd32(&fatSector[(cluster * 4) % EMMC_BLOCK_SIZE]) & FAT32_MASK;
}

static void Fat32_ParseEntry(const uint8_t *d, Fat32Entry_t *e)
{
    int n = 0;
    for(int i = 0; i < 8 && d[i] != ' '; i++)
        e->name[n++] = Fat32_ToLower(d[i]);
    if(d[8] != ' ')
    {
        e->name[n++] = '.';
        for(int i = 8; i < 11 && d[i] != ' '; i++)
            e->name[n++] = Fat32_ToLower(d[i]);
    }
    e->name[n] = 0;

    e->attr = d[11];
    e->cluster = ((uint32_t)rd16(&d[20]) << 16) | rd16(&d[26]);
    e->size = rd32(&d[28]);
}

int Fat32_Mount(Fat32_t *fs)
{
    if(EMMC_ReadBlocks(0, 1, tmpSector) != 0 || rd16(&tmpSector[510]) != 0xAA55)
    {
        ERROR("[fat] no MBR on SD\n");
        return -1;
    }

    uint32_t partLba = 0;
    for(int i = 0; i < 4; i++)
    {
        uint8_t *part = &tmpSector[446 + i * 16];
        if(part[4] == 0x0B || part[4] == 0x0C)
        {
            partLba = rd32(&part[8]);
            break;
        }
    }

    if(partLba && EMMC_ReadBlocks(partLba, 1, tmpSector) != 0)
        return -1;

    uint32_t spc = tmpSector[13];
    uint32_t reserved = rd16(&tmpSector[14]);
    uint32_t numFats = tmpSector[16];
    uint32_t totalSectors = rd32(&tmpSector[32]);
    uint32_t fatSize = rd32(&tmpSector[36]);

    //FAT12/16 keep their FAT size in the 16-bit field and have no root cluster
    if(rd16(&tmpSector[510]) != 0xAA55 || rd16(&tmpSector[11]) != EMMC_BLOCK_SIZE ||
        spc == 0 || (spc & (spc - 1)) || numFats == 0 || rd16(&tmpSector[22]) != 0 || fatSize == 0)
    {
        ERROR("[fat] no FAT32 volume @ lba 0x%X\n", partLba);
        return -1;
    }

    fs->fatLba = partLba + reserved;
    fs->dataLba = fs->fatLba + numFats * fatSize;
    fs->sectorsPerCluster = spc;
    fs->rootCluster = rd32(&tmpSector[44]);

    uint32_t metaSectors = fs->dataLba - partLba;
    if(totalSectors <= metaSectors)
        return -1;
    fs->clusterCount = (totalSectors - metaSectors) / spc;
    fatSectorLba = 0xFFFFFFFF;

    LOG("[fat] FAT32 @ lba 0x%X [%d clusters]\n", partLba, fs->clusterCount);
    return 0;
}

int Fat32_ListDir(Fat32_t *fs, uint32_t dirCluster, Fat32_Visitor visit, void *ctx)
{
    uint32_t cluster = dirCluster;

    //Hop limit guards against loops in a corrupt FAT
    for(uint32_t hops = 0; Fat32_IsValidCluster(fs, cluster) && hops < fs->clusterCount; hops++)
    {
        uint32_t lba = Fat32_ClusterLba(fs, cluster);
        for(uint32_t s = 0; s < fs->sectorsPerCluster; s++)
        {
            if(EMMC_ReadBlocks(lba + s, 1, dirSector) != 0)
                return -1;

            for(uint32_t off = 0; off < EMMC_BLOCK_SIZE; off += DIRENT_SIZE)
            {
                uint8_t *d = &dirSector[off];
                if(d[0] == 0x00)
                    return 0; //end of directory
                if(d[0] == 0xE5 || d[11] == ATTR_LFN || (d[11] & ATTR_VOLUME_ID))
                    continue;

                Fat32Entry_t entry;
                Fat32_ParseEntry(d, &entry);
                if(visit(fs, &entry, ctx))
                    return 0;
            }
        }
        cluster = Fat32_NextCluster(fs, cluster);
    }

    return cluster >= FAT32_EOC ? 0 : -1;
}

typedef struct
{
    const char *name;
    Fat32Entry_t *out;
    int found;
} Fat32FindCtx_t;

static int Fat32_FindVisitor(Fat32_t *fs, Fat32Entry_t *entry, void *ctx)
{
    UNUSED(fs);
    Fat32FindCtx_t *find = (Fat32FindCtx_t*)ctx;
    if(!Fat32_StrEq(entry->name, find->name))
        return 0;

    *find->out = *entry;
    find->found = 1;
    return 1;
}

int Fat32_Find(Fat32_t *fs, uint32_t dirCluster, const char *name, Fat32Entry_t *out)
{
    Fat32FindCtx_t find = { name, out, 0 };
    if(Fat32_ListDir(fs, dirCluster, Fat32_FindVisitor, &find) != 0)
        return -1;
    return find.found ? 0 : -1;
}

int Fat32_Open(Fat32_t *fs, const char *path, Fat32Entry_t *out)
{
    //Start at the root, which has no directory entry of its own
    Fat32Entry_t cur;
    cur.name[0] = 0;
    cur.attr = FAT_ATTR_DIRECTORY;
    cur.cluster = fs->rootCluster;
    cur.size = 0;

    while(*path)
    {
        while(*path == '/')
            path++;
        if(!*path)
            break;

        char comp[13];
        int n = 0;
        for(; *path && *path != '/'; path++)
        {
            if(n >= (int)sizeof(comp) - 1)
                return -1;
            comp[n++] = *path;
        }
        comp[n] = 0;

        if(!(cur.attr & FAT_ATTR_DIRECTORY) || Fat32_Find(fs, cur.cluster, comp, &cur) != 0)
            return -1;
    }

    *out = cur;
    return 0;
}

long Fat32_ReadAt(Fat32_t *fs, Fat32Entry_t *entry, uint32_t offset, uint8_t *buf, uint32_t size)
{
    if(offset >= entry->size)
        return 0;
    if(size > entry->size - offset)
        size = entry->size - offset;

    uint32_t clusterBytes = fs->sectorsPerCluster * EMMC_BLOCK_SIZE;
    uint32_t cluster = entry->cluster;
    uint32_t hops = 0;

    for(uint32_t skip = offset / clusterBytes; skip; skip--, hops++)
    {
        if(!Fat32_IsValidCluster(fs, cluster) || hops >= fs->clusterCount)
            return -1;
        cluster = Fat32_NextCluster(fs, cluster);
    }

    uint32_t pos = offset % clusterBytes;
    uint32_t done = 0;
    while(done < size)
    {
        if(!Fat32_IsValidCluster(fs, cluster))
            return -1;

        uint32_t secOff = pos % EMMC_BLOCK_SIZE;
        if(EMMC_ReadBlocks(Fat32_ClusterLba(fs, cluster) + pos / EMMC_BLOCK_SIZE, 1, tmpSector) != 0)
            return -1;

        uint32_t n = EMMC_BLOCK_SIZE - secOff;
        if(n > size - done)
            n = size - done;
        for(uint32_t i = 0; i < n; i++)
            buf[done + i] = tmpSector[secOff + i];
        done += n;
        pos += n;

        if(pos == clusterBytes)
        {
            if(++hops >= fs->clusterCount)
                return -1;
            pos = 0;
            cluster = Fat32_NextCluster(fs, cluster);
        }
    }

    return done;
}
