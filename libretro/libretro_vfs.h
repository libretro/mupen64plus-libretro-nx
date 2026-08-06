#ifndef _LIBRETRO_VFS_H_
#define _LIBRETRO_VFS_H_

#include <stdbool.h>

#include "libretro.h"

extern struct retro_vfs_interface *vfs_interface;

void *vfs_fopen(const char *path, bool write);
bool vfs_file_exists(const char *path);
void vfs_fclose(void *fp);

#endif
