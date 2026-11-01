#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

//FatFs and dirent.h both define DIR
#define DIR FF_DIR
#include "ff.h"
#include "diskio.h"
#undef DIR

#define SECTOR_SIZE 512

PARTITION VolToPart[FF_VOLUMES] = { {0, 0}, {0, 0} };

static int imageFd;

DSTATUS disk_status(BYTE pdrv)
{
    return 0;
}

DSTATUS disk_initialize(BYTE pdrv)
{
    return 0;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count)
{
    size_t size = (size_t)count * SECTOR_SIZE;
    return pread(imageFd, buff, size, (off_t)sector * SECTOR_SIZE) == (ssize_t)size ? RES_OK : RES_ERROR;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count)
{
    size_t size = (size_t)count * SECTOR_SIZE;
    return pwrite(imageFd, buff, size, (off_t)sector * SECTOR_SIZE) == (ssize_t)size ? RES_OK : RES_ERROR;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    return RES_OK;
}

static void Fail(const char *what, const char *path)
{
    fprintf(stderr, "mkrootfs: %s failed for %s\n", what, path);
    exit(1);
}

static void CopyFile(const char *src, const char *dst)
{
    static char buf[0x10000];
    static FIL file;

    int fd = open(src, O_RDONLY);
    if(fd < 0)
        Fail("open", src);
    if(f_open(&file, dst, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
        Fail("create", dst);

    ssize_t n;
    while((n = read(fd, buf, sizeof(buf))) > 0)
    {
        UINT written = 0;
        if(f_write(&file, buf, n, &written) != FR_OK || written != (UINT)n)
            Fail("write", dst);
    }
    if(n < 0)
        Fail("read", src);

    if(f_close(&file) != FR_OK)
        Fail("close", dst);
    close(fd);
}

static void CopyDir(const char *src, const char *dst)
{
    struct dirent **names;
    int count = scandir(src, &names, 0, alphasort);
    if(count < 0)
        Fail("scandir", src);

    for(int i = 0; i < count; i++)
    {
        const char *name = names[i]->d_name;
        if(strcmp(name, ".") != 0 && strcmp(name, "..") != 0 && strcmp(name, ".keep") != 0)
        {
            char srcPath[1024], dstPath[1024];
            snprintf(srcPath, sizeof(srcPath), "%s/%s", src, name);
            snprintf(dstPath, sizeof(dstPath), "%s/%s", dst, name);

            struct stat st;
            if(stat(srcPath, &st) != 0)
                Fail("stat", srcPath);

            if(S_ISDIR(st.st_mode))
            {
                if(f_mkdir(dstPath) != FR_OK)
                    Fail("mkdir", dstPath);
                CopyDir(srcPath, dstPath);
            }
            else
                CopyFile(srcPath, dstPath);
        }
        free(names[i]);
    }
    free(names);
}

int main(int argc, char **argv)
{
    static FATFS fs;
    if(argc != 3)
    {
        fprintf(stderr, "usage: mkrootfs <image> <dir>\n");
        return 1;
    }

    imageFd = open(argv[1], O_RDWR);
    if(imageFd < 0)
        Fail("open", argv[1]);
    if(f_mount(&fs, "0:", 1) != FR_OK)
        Fail("mount", argv[1]);

    CopyDir(argv[2], "0:");

    if(f_unmount("0:") != FR_OK)
        Fail("unmount", argv[1]);
    close(imageFd);
    return 0;
}
