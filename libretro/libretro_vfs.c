#include "libretro_vfs.h"

#include <stdio.h>
#include <stdlib.h>

struct retro_vfs_interface *vfs_interface = NULL;

void *vfs_fopen(const char *path, bool write)
{
    if (vfs_interface)
    {
        unsigned access = write ? RETRO_VFS_FILE_ACCESS_WRITE
                                 : RETRO_VFS_FILE_ACCESS_READ;
        return (void*)vfs_interface->open(path, access, RETRO_VFS_FILE_ACCESS_HINT_NONE);
    }

    return (void*)fopen(path, write ? "wb" : "rb");
}

bool vfs_file_exists(const char *path)
{
    void *fp = vfs_fopen(path, false);
    if (!fp)
        return false;

    vfs_fclose(fp);
    return true;
}

void vfs_fclose(void *fp)
{
    if (!fp)
        return;

    if (vfs_interface)
        vfs_interface->close((struct retro_vfs_file_handle*)fp);
    else
        fclose((FILE*)fp);
}
